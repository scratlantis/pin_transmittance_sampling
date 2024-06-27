#version 460
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

struct Params
{
    ivec2 offset;
	uvec2 extent;
	uint count;
};
layout(binding = 0) uniform PARAMS {Params params;};
layout(binding = 1) uniform sampler uSampler;
layout(binding = 2) uniform texture2D uTex;


void main()
{
	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	ivec2 texelPos = ivec2(params.offset + inUV * (params.extent));
	vec4 texColor = texelFetch(sampler2D(uTex, uSampler), texelPos, 0);
	outColor.rgb = texColor.rgb;
	//outColor.r = 1.0;
	outColor.a = 1.0/float(params.count);
}