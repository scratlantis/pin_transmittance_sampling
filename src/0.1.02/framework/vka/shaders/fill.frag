#version 460
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform COLOR_CONST {vec4 uColor;};

void main()
{
	outColor = uColor;
}