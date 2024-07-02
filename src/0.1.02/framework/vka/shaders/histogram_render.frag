#version 460
#extension GL_GOOGLE_include_directive : enable

#ifndef BUCKETS
#define BUCKETS 256
#endif

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