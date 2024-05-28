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
layout(binding = 3) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};

void main()
{
	//vec3 worldPos = (fragment_modelMat*vec4(fragment_position,1.0)).xyz;
	//vec3 dirWorldSpace = normalize(worldPos-view.camPos.xyz);
	//vec3 dir = (fragment_invModelMat * vec4(dirWorldSpace,0.0)).xyz;
	//dir = normalize(dir);

	vec3 dir = normalize(-fragment_position);
	dir = (view.inverseViewMat * vec4(dir, 0.0)).xyz;
	vec3 startPos = view.probe.xyz;
	startPos = (view.fogInvModelMatrix*vec4(startPos,1.0)).xyz;
	startPos = clamp(startPos, vec3(0.0), vec3(1.0));
	vec3 p1 = startPos;
	//vec3 p1 = max(vec3(0.0,0.0,0.0),(fragment_position + EPSILON * dir));
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



	if(true)
	{
		vec3 origin,direction;
		Pin p = grid[maxDotIdx].pin;
		getRay(p, origin, direction);
		dir = direction * sign(dot(dir, direction));
		startPos = origin + dir*dot(startPos-origin,dir);
		assureNotZero(dir);
		vec3 invDir = 1.0 / dir;
		vec3 c1 = -startPos*invDir;
		vec3 c2 = c1 + invDir; // (vec(1.0)-raySeg.origin)*invDir;
		float tMin = max(max(min(c1.x, c2.x), min(c1.y, c2.y)), min(c1.z, c2.z));
		float tMax = min(min(max(c1.x, c2.x), max(c1.y, c2.y)), max(c1.z, c2.z));
		vec3 entryPoint = startPos;//fragment_position + tMin*dir;
		vec3 exitPoint = startPos + tMax*dir;
		transmittance = 1.0;
		float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
		for(int i = 0; i < GAUSSIAN_COUNT; i++)
		{
			float coef = clamp(1.0-weight*evalTransmittanceGaussianSegment(entryPoint, exitPoint, gaussians[i]), 0.0, 1.0);
			transmittance *= coef;
		}
	}



	outColor = vec4(transmittance, transmittance, transmittance, 1.0);
}