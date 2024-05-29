#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_control_flow_attributes : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 1) flat in int fragment_instanceId;
layout(location = 2) in mat4 fragment_modelMat;
layout(location = 6) in mat4 fragment_invModelMat;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer PINS {Pin pins[PIN_COUNT];};
layout(binding = 2) buffer PIN_TRANSMITTANCE {float pin_transmittance[PIN_COUNT];};
layout(binding = 3) buffer PIN_DIRECTION {vec4 pin_dir[PIN_COUNT];};
layout(binding = 4) buffer PINS_USED {uint pinsUsed[PIN_COUNT];};
layout(binding = 5) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};
void main()
{
	vec3 pos = vec3(-fragment_position.x, fragment_position.y, -fragment_position.z);
	vec3 dir = (view.inverseViewMat * vec4(normalize(pos), 0.0)).xyz;
	vec3 startPos = clamp( (view.fogInvModelMatrix*vec4(view.probe.xyz,1.0)).xyz ,vec3(0.0), vec3(1.0) );
	

	float maxDot = 0.0;
	uint maxDotIdx = 0;

	uint offset = fragment_instanceId * PIN_COUNT_SQRT;

	[[unroll]]
	for(uint i = 0; i < PIN_COUNT_SQRT; i++)
	{
		vec3 origin,direction;
		Pin p = pins[i+offset];
		getRay(p, origin, direction);
		float dotProd = abs(dot(dir, direction));
		float dist = 1.0-distance(origin,startPos);
		dotProd = mix(dotProd, dist, view.pinSelectionCoef);
		if(dotProd > maxDot)
		{
			maxDot = dotProd;
			maxDotIdx = i+offset;
		}
		pinsUsed[i+offset] = 0;
	}
	pinsUsed[maxDotIdx] = 1;

	gl_FragDepth = 1.0-maxDot;
	vec3 origin,direction;
	Pin p = pins[maxDotIdx];
	getRay(p, origin, direction);
	dir = direction * sign(dot(dir, direction));
	startPos = origin + dir*dot(startPos-origin,dir);

	vec3 endPos = cubeExitPoint(startPos, dir);
	float transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-weight*evalTransmittanceGaussianSegment(startPos, endPos, gaussians[i]), 0.0, 1.0);
	}

	outColor = vec4(vec3(transmittance), 1.0);

}