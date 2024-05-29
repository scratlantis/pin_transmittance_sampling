#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_control_flow_attributes : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 1) flat in int fragment_instanceId;
layout(location = 2) in mat4 fragment_modelMat;
layout(location = 6) in mat4 fragment_invModelMat;

layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outPinId;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer PINS {Pin pins[PIN_COUNT];};
layout(binding = 2) buffer PIN_TRANSMITTANCE {float pin_transmittance[PIN_COUNT];};
layout(binding = 3) buffer PIN_DIRECTION {vec4 pin_dir[PIN_COUNT];};
layout(binding = 4) buffer PINS_USED {uint pinsUsed[PIN_COUNT];};
layout(binding = 5) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};

#ifndef METRIC_DISTANCE_DISTANCE
	#ifndef METRIC_ANGLE_DISTANCE
		#define METRIC_ANGLE_DISTANCE
	#endif
#endif

void main()
{
	vec3 pos = vec3(-fragment_position.x, fragment_position.y, -fragment_position.z);
	vec3 dir = (view.inverseViewMat * vec4(normalize(pos), 0.0)).xyz;
	vec3 startPos = clamp( (view.fogInvModelMatrix*vec4(view.probe.xyz,1.0)).xyz ,vec3(0.0), vec3(1.0) );
	applyJitter(view.positionalJitter, view.angularJitter, startPos, dir);
	vec3 endPos = startPos+dir*view.secRayLength;


	float maxMetric = 0.0;
	uint maxMetricIdx = 0;

	uint offset = fragment_instanceId * PIN_COUNT_SQRT;

	[[unroll]]
	for(uint i = 0; i < PIN_COUNT_SQRT; i++)
	{
		vec3 origin,direction;
		Pin p = pins[i+offset];
		getRay(p, origin, direction);


		vec3 closestPoint = origin + direction*dot(startPos-origin,direction);
		float dist = distance(closestPoint, startPos);
		float invDist = 1.0/(dist+1.0);

		#ifdef METRIC_ANGLE_DISTANCE
		float dotProd = abs(dot(dir, direction));
		float metric = mix(dotProd, invDist, view.pinSelectionCoef);
		#endif
		#ifdef METRIC_DISTANCE_DISTANCE
		vec3 closestPointEnd = origin + direction*dot(endPos-origin,direction);
		float distEnd = distance(closestPointEnd, endPos);
		float invDistEnd = 1.0/(distEnd+1.0);
		float metric = mix(invDistEnd, invDist, view.pinSelectionCoef);
		#endif

		if(metric > maxMetric)
		{
			maxMetric = metric;
			maxMetricIdx = i+offset;
		}
		pinsUsed[i+offset] = 0;
	}
	//pinsUsed[maxMetricIdx] = 1;

	outPinId = maxMetricIdx+1;

	gl_FragDepth = 1.0-maxMetric;
	vec3 origin,direction;
	Pin p = pins[maxMetricIdx];
	getRay(p, origin, direction);
	dir = direction * sign(dot(dir, direction));
	startPos = origin + dir*dot(startPos-origin,dir);

	vec3 exitPoint = cubeExitPoint(startPos, dir);
	endPos = exitPoint;
	if(distance(startPos, exitPoint) > view.secRayLength)
	{
		endPos = startPos + dir*view.secRayLength;
	}

	float transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-weight*evalTransmittanceGaussianSegment(startPos, endPos, gaussians[i]), 0.0, 1.0);
	}

	outColor = vec4(vec3(transmittance), 1.0);

}