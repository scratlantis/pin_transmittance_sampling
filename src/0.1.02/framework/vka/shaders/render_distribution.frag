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
layout(binding = 1) buffer COUNTER_BUFFER {float bucket[BUCKETS];};
void main()
{
	uint index = uint(inUV.x * BUCKETS);
	float normalizationTerm = 1.0f;
	index = clamp(index, 0u, BUCKETS - 1u);
	float height = bucket[index] * normalizationTerm;
	if(1.0 - inUV.y < height)
	{
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		outColor = vec4(0.0, 0.0, 0.0, 0.3);
	}
}