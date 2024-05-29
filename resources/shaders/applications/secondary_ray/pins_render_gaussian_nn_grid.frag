#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 1) flat in int fragment_instanceId;
layout(location = 2) in mat4 fragment_modelMat;
layout(location = 6) in mat4 fragment_invModelMat;

layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outPinId;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer PIN_TRANSMITTANCE {float pin_transmittance[PIN_COUNT];};
layout(binding = 2) buffer PIN_GRID {PinGridEntry grid[PIN_GRID_SIZE*PIN_GRID_SIZE*PIN_GRID_SIZE*PINS_PER_GRID_CELL];};
layout(binding = 3) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};

void main()
{

	vec3 pos = vec3(-fragment_position.x, fragment_position.y, -fragment_position.z);
	vec3 dir = (view.inverseViewMat * vec4(normalize(pos), 0.0)).xyz;
	vec3 startPos = clamp( (view.fogInvModelMatrix*vec4(view.probe.xyz,1.0)).xyz ,vec3(0.0), vec3(1.0) );

	vec3 p1 = startPos;
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

	vec3 origin,direction;
	Pin p = grid[maxDotIdx].pin;
	getRay(p, origin, direction);
	dir = direction * sign(dot(dir, direction));
	startPos = origin + dir*dot(startPos-origin,dir);
	vec3 endPos = cubeExitPoint(startPos, dir);

	transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-weight*evalTransmittanceGaussianSegment(startPos, endPos, gaussians[i]), 0.0, 1.0);
	}


	outPinId=0;
	outColor = vec4(vec3(transmittance), 1.0);
}