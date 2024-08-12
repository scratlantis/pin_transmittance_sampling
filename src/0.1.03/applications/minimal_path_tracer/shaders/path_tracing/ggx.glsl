
https://anderslanglands.github.io/


// This data struct would be filled out in the material init function
struct BsdfData {
    float4 rho;              // reflectance
    float alpha_u;           // roughness in u direction
    float alpha_v;           // roughness in v direction
    float r0;                // normal reflectance
    float weight;            // 
    unsigned char bsdf_type; // 
};

// NDF
float D_ggx(
    const float3& m,        // microfacet normal
    const float alpha_x,    // roughness in x direction
    const float alpha_y     // roughness in v direction
    ) {
    const float sx = -m.x / (m.z * alpha_x);
    const float sy = -m.y / (m.z * alpha_y);
    const float sl = 1.0f + sx * sx + sy * sy;
    const float cos_theta_m4 = m.z * m.z * m.z * m.z;
    return 1.0f / ((sl * sl) * RA_PI * alpha_x * alpha_y * cos_theta_m4);
}

// Smith masking function
float G_smith(
    const float3& omega,   // incident/exitant direction
    const float ax2,       // x roughness^2
    const float ay2        // y roughness^2
    ) {
    const float cos_o2 = omega.z * omega.z;
    const float tan_theta_o2 = (1.0f - cos_o2) / cos_o2;
    const float cos_phi_o2 = omega.x * omega.x;
    const float sin_phi_o2 = omega.y * omega.y;

    const float alpha_o2 =
        (cos_phi_o2 * ax2 + sin_phi_o2 * ay2) / (cos_phi_o2 + sin_phi_o2);

    return 2.0f / (1.0f + safe_sqrtf(1.0f + alpha_o2 * tan_theta_o2));
}

// Sample a visible normal as per Heitz 2018: 
// http://jcgt.org/published/0007/04/01/
float3 sample_visible_normal_ggx(
    const float3& omega_o_l,    // the outgoing direction
    const float ax,             // roughness in x
    const float ay,             // roughness in y
    const float x1,             // sample
    const float x2              // sample
    ) {
    const float3 v_h =
        normalize(make_float3(ax * omega_o_l.x, ay * omega_o_l.y, omega_o_l.z));
    // orthonormal basis
    const float lensq = v_h.x * v_h.x + v_h.y * v_h.y;
    const float3 T1 = lensq > 0 ? make_float3(-v_h.y, v_h.x, 0.0f) / sqrtf(lensq)
                          : make_float3(1, 0, 0);
    const float3 T2 = cross(v_h, T1);
    // parameterization of projected area
    const float r = sqrtf(x1);
    const float phi = 2.0f * RA_PI * x2;
    const float t1 = r * cosf(phi);
    float t2 = r * sinf(phi);
    const float s = 0.5f * (1.0f * v_h.z);
    t2 = (1.0f - s) * safe_sqrtf(1.0f - t1 * t1) + s * t2;
    // reprojection onto hemisphere
    const float3 n_h = 
        t1 * T1 + t2 * T2 + safe_sqrtf(1.0f - t1 * t1 - t2 * t2) * v_h;
    // transform back to ellipsoid
    return normalize(make_float3(ax * n_h.x, ay * n_h.y, max(0.0f, n_h.z)));
}

void bsdf_eval(
    BsdfData& bsdf_data,        // data struct with BDSF parameters
    const float3& omega_o_l,    // exitant direction
    const float3& omega_i_l,    // incident direction
    const float4& lambda,       // active wavelengths for current path
    float4& f,                  // BSDF result
    float4& kt,                 // complement of fresnel reflection
                                // for material layering
    float4& pdf                 // probability of sampling omega_i_l
    ) {
    if (omega_i_l.z <= 0.0f) {
        f = make_float4(0.0f);
        pdf = make_float4(0.0f);
        return;
    }

    const float alpha_x = fmaxf(1.0e-7f, bsdf_data.alpha_u);
    const float alpha_y = fmaxf(1.0e-7f, bsdf_data.alpha_v);

    const float ax2 = alpha_x * alpha_x;
    const float ay2 = alpha_y * alpha_y;

    // microfacet normal
    const float3 m = normalize(omega_o_l + omega_i_l);
    const float mu_m = max(0.0, dot(omega_o_l, m));

    // normal distribution function
    const float D = D_ggx(m, alpha_x, alpha_y);

    // masking-shadowing
    const float G1_o = G_smith(omega_o_l, ax2, ay2);
    const float G1_i = G_smith(omega_i_l, ax2, ay2);
    const float G2 = G1_o * G1_i;

    // fresnel
    const float kr = schlick_fresnel(data.r0, mu_m);
    kt = make_float4(1.0f - kr);

    const float denom = 4.0f * omega_i_l.z * omega_o_l.z;

    f = bsdf_data.rho * D * G2 * kr / denom;

    pdf = make_float4(G1_o * D * mu_m / denom);
}

float4 bsdf_pdf(
    BsdfData& bsdf_data, 
    const float3& omega_o_l,
    const float3& omega_i_l, 
    const float4& lambda
    ) {
    if (omega_i_l.z <= 0.0f) {
        return make_float4(0.0f);
    }

    const float alpha_x = fmaxf(1.0e-7f, bsdf_data.alpha_u);
    const float alpha_y = fmaxf(1.0e-7f, bsdf_data.alpha_v);

    const float ax2 = alpha_x * alpha_x;
    const float ay2 = alpha_y * alpha_y;

    // microfacet normal
    const float3 m = normalize(omega_o_l + omega_i_l);
    const float mu_m = max(0.0, dot(omega_o_l, m));

    // normal distribution function
    const float D = D_ggx(m, alpha_x, alpha_y);

    // masking-shadowing
    const float G1_o = G_smith(omega_o_l, ax2, ay2);

    const float denom = 4.0f * omega_i_l.z * omega_o_l.z;

    return make_float4(G1_o * D * mu_m / denom);
}

void bsdf_sample(BsdfData& bsdf_data,
                                     const float3& omega_o_l,
                                     const float4& lambda, const float x1,
                                     const float x2, float3& omega_i_l,
                                     float4& f, float4& kt, float4& pdf) {
    const float ax = fmaxf(1.0e-7f, bsdf_data.alpha_u);
    const float ay = fmaxf(1.0e-7f, bsdf_data.alpha_v);

    // sample microfacet normal
    const float3 m = sample_visible_normal_ggx(omega_o_l, ax, ay, x1, x2);
    const float mu_m = max(0.0, dot(omega_o_l, m));

    // reflect to get incoming direction
    omega_i_l = reflect(-omega_o_l, m);

    const float ax2 = ax * ax;
    const float ay2 = ay * ay;

    // normal distribution function
    const float D = D_ggx(m, ax, ay);

    // masking-shadowing
    const float G1_o = G_smith(omega_o_l, ax2, ay2);
    const float G1_i = G_smith(omega_i_l, ax2, ay2);
    const float G2 = G1_o * G1_i;

    // fresnel
    const float kr = schlick_fresnel(data.r0, mu_m);
    kt = make_float4(1.0f - kr);

    const float denom = 4.0f * omega_i_l.z * omega_o_l.z;

    f = bsdf_data.rho * D * G2 * kr / denom;
    pdf = make_float4(G1_o * D * mu_m / denom);
}