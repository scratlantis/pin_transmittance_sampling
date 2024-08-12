
// GGX Smith Microfacet brdf for specular component, lampert for diffuse, Fresnel to decide

// Sampling the GGX Distribution of Visible Normals
// http://jcgt.org/published/0007/04/01/

// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
vec3 sampleGGXVNDF(vec3 Ve, float alpha_x, float alpha_y, float U1, float U2)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	vec3 Vh = normalize(vec3(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));
	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	vec3 T1 = lensq > 0 ? vec3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : vec3(1,0,0);
	vec3 T2 = cross(Vh, T1);
	// Section 4.2: parameterization of the projected area
	float r = sqrt(U1);
	float phi = 2.0 * PI * U2;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + Vh.z);
	t2 = (1.0 - s)*sqrt(1.0 - t1*t1) + s*t2;
	// Section 4.3: reprojection onto hemisphere
	vec3 Nh = t1*T1 + t2*T2 + sqrt(max(0.0, 1.0 - t1*t1 - t2*t2))*Vh;
	// Section 3.4: transforming the normal back to the ellipsoid configuration
	vec3 Ne = normalize(vec3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0, Nh.z)));
	return Ne;
}



float GGX_lambda(vec3 V, vec2 a)
{
	vec2 a_square = a*a;
	vec3 V_square = V*V;
	return (-1 + sqrt(1 + (a_square.x * V_square.x + a_square.y * V_square.y)/V_square.z))/2.0f;
}

// Smith shadowing function
float SmithG1(vec3 V, vec2 a)
{
	return 1 / (1 + GGX_lambda(V, a));
}

// Smith shadowing masking function
float SmithG2(vec3 V, vec3 L, vec2 a)
{
	return 1 / (1 + GGX_lambda(V, a) + GGX_lambda(L, a));
}


float weightGGXVNDF(vec3 V, vec3 L, vec2 a)
{
	return SmithG2(V, L, a)/SmithG1(V,a);
}

vec3 sampleLampert(vec2 xi)
{
	float xi_0_sqrt = sqrt(xi.x);
	float phi = 2.0f*PI*xi.y;
	return vec3(cos(phi) * xi_0_sqrt, sin(phi) * xi_0_sqrt, sqrt(1.0f - xi.x));
}

float weightLampert()
{
	return 1.0;
}