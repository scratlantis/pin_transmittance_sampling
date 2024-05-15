#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};
layout(location = 1) in vec3 fragment_color;

void main()
{
	outColor = vec4(fragment_color,1.0);
}