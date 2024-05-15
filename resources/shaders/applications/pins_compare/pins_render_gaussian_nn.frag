#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};
layout(binding = 2) buffer PINS {Pin pins[PIN_COUNT];};
layout(binding = 3) buffer PIN_TRANSMITTANCE {float pin_transmittance[PIN_COUNT];};


void main()
{
	outColor = vec4(1.0,0.0,0.0,1.0);
}