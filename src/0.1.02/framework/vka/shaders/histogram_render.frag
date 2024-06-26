#version 460
#extension GL_GOOGLE_include_directive : enable

#ifndef BUCKETS
#define BUCKETS 256
#endif

	float gauss_cdf_approx_Eidous_Ananbeh(float z)
{
	float c1 = 1.5957764;
	float c2 = 0.0726161;
	float c3 = 0.00003318;
	float c4 = 0.00021785;
	float c5 = 0.00006293;
	float c6 = 0.00000519;
	float z3 = z * z * z;
	float z6 = z3*z3;
	float z7 = z6*z;
	float z8 = z7*z;
	float z9 = z8*z;
	return 1.0 - 1.0 / (1.0 + exp(c1*z + c2*z3+c3*z6+c4*z7+c5*z8+c6*z9));
}

float gauss_inv_cdf_approx_Schmeiser(float pIn)
{
	float coef = 5.06329113924; // 1.0 / 0.1975
	if(pIn>=0.5)
	{
		float p = pIn;
		return (pow(p, 0.135) - pow(1.0 - p, 0.135)) * coef;
	}
	else
	{
		float p = 1.0-pIn;
		return - (pow(p, 0.135) - pow(1.0 - p, 0.135)) * coef;
	}
}

struct Params
{
    ivec2 offset;
	uvec2 extent;
};
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform PARAMS {Params params;};
layout(binding = 1) buffer COUNTER_BUFFER {uint bucket[BUCKETS];};
layout(binding = 2) buffer AVERAGE_BUFFER {float average;};
void main()
{
	uint index = uint(inUV.x * BUCKETS);
	float normalizationTerm = 10000.0 / float(params.extent.x * params.extent.y * BUCKETS);
	index = clamp(index, 0u, BUCKETS - 1u);
	uint val = bucket[index];
	float height = float(val)*normalizationTerm;

	//height = 0.5 + 0.1*gauss_inv_cdf_approx_Schmeiser(inUV.x);
	//height = abs(gauss_inv_cdf_approx_Schmeiser(0.4999));
	if(abs(inUV.x - average) < 0.01)
	{
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
	else if(1.0 - inUV.y < height)
	{
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		outColor = vec4(0.0, 0.0, 0.0, 0.3);
	}




	//outColor.xyz = vec3(height,0.0,0.0);
}