#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer PIN_TRANSMITTANCE {float pin_transmittance[PIN_COUNT];};
layout(binding = 2) buffer PIN_GRID {PinGridEntry grid[PIN_GRID_SIZE*PIN_GRID_SIZE*PIN_GRID_SIZE*PINS_PER_GRID_CELL];};



void main()
{
	outColor = vec4(1.0,0.0,0.0,1.0);
}