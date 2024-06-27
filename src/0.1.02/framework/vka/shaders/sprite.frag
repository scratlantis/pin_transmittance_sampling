#version 460
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;



layout(binding = 0) uniform sampler uSampler;
layout(binding = 1) uniform texture2D uTex;



void main()
{
	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 texColor = texture(sampler2D(uTex, uSampler), inUV);
	outColor.rgb = texColor.rgb;
}