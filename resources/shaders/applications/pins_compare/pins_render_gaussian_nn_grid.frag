#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 1) flat in int fragment_instanceId;
layout(location = 2) in mat4 fragment_modelMat;
layout(location = 6) in mat4 fragment_invModelMat;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer PIN_TRANSMITTANCE {float pin_transmittance[PIN_COUNT];};
layout(binding = 2) buffer PIN_GRID {PinGridEntry grid[PIN_GRID_SIZE*PIN_GRID_SIZE*PIN_GRID_SIZE*PINS_PER_GRID_CELL];};


void main()
{
	vec3 worldPos = (fragment_modelMat*vec4(fragment_position,1.0)).xyz;
	vec3 dirWorldSpace = normalize(worldPos-view.camPos.xyz);
	vec3 dir = (fragment_invModelMat * vec4(dirWorldSpace,0.0)).xyz;
	dir = normalize(dir);

	vec3 p1 = max(vec3(0.0,0.0,0.0),(fragment_position + EPSILON * dir));
	uvec3 p1ID = uvec3(floor(p1*PIN_GRID_SIZE));
	uint gridIdx = getCellIndex(p1ID);
	float maxDot = 0.0;
	uint maxDotIdx = gridIdx;
	for(uint i = 0; i < PINS_PER_GRID_CELL; i++)
	{
		Pin pin = grid[gridIdx + i].pin;
		vec3 origin,direction;
		getRay(pin, origin, direction);
		float dotProd = dot(direction, dir);
		if(dotProd > maxDot)
		{
			maxDot = dotProd;
			maxDotIdx = i + gridIdx;
		}
	}
	uint pinIdx = grid[maxDotIdx].data.pinIndex;
	float transmittance = pin_transmittance[pinIdx];
	outColor = vec4(transmittance-0.1, transmittance, transmittance, 1.0);
}