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
	//vec3 worldPos = (fragment_modelMat*vec4(fragment_position,1.0)).xyz;
	//vec3 dirWorldSpace = normalize(worldPos-view.camPos.xyz);
	//vec3 dir = (fragment_invModelMat * vec4(dirWorldSpace,0.0)).xyz;
	//dir = normalize(dir);

	vec3 dir = normalize(-fragment_position);
	dir = (view.inverseViewMat * vec4(dir, 0.0)).xyz;
	vec3 startPos = view.probe.xyz;
	startPos = (view.fogInvModelMatrix*vec4(startPos,1.0)).xyz;
	startPos = clamp(startPos, vec3(0.0), vec3(1.0));

	float maxDot = 0.0;
	uint maxDotIdx = 0;

	uint offset = fragment_instanceId * PIN_COUNT_SQRT;

	//float theta = acos(dir.z)/PI;
	//float phi = 0.5+0.5*sign(dir.y)*acos(dir.x/length(dir.xy))/(PI);

	
	[[unroll]]
	for(uint i = 0; i < PIN_COUNT_SQRT; i++)
	{
		vec3 origin,direction;
		Pin p = pins[i+offset];
		getRay(p, origin, direction);
		//float dotProd = abs(dot(dir, pin_dir[i+offset].xyz));
		float dotProd = abs(dot(dir, direction));
		float dist = 1.0-distance(origin,startPos);
		dotProd = mix(dotProd, dist, view.pinSelectionCoef);
		//deltaPhi = min(abs(deltaPhi), abs(deltaPhi+PI));
		//deltaTheta = min(abs(deltaTheta), abs(deltaTheta+2*PI));
	
		//float dist = dotProd;//sqrt(deltaPhi*deltaPhi+deltaTheta*deltaTheta);
	
		//dotProd = 1.0-dist;//mix(dotProd, dist, 1.0);
	
		if(dotProd > maxDot)
		{
			maxDot = dotProd;
			maxDotIdx = i+offset;
		}
		pinsUsed[i+offset] = 0;
	}
	pinsUsed[maxDotIdx] = 1;

	//float deltaTheta = min(abs(theta), abs(theta));
	gl_FragDepth = 1.0-maxDot;
	float invTransmittance = 1.0-pin_transmittance[maxDotIdx];
	float transmittance = 1.0-invTransmittance;



	if(true)
	{
		vec3 origin,direction;
		Pin p = pins[maxDotIdx];
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






	//transmittance*=0.1;
	//transmittance=0.0;
	outColor = vec4(transmittance, transmittance, transmittance, 1.0);
	//outColor = vec4(0.0, 0.0, 0.0, 1.0);
	//outColor = vec4(maxDot*0.2, maxDot*0.2, maxDot*0.2, 1.0);
	//vec3 color = vec3(phi);
	//outColor = vec4(color, 1.0);

	//float deltaPhi = abs(pins[i+offset].x - phi);
	//outColor = vec4(deltaPhi/(PI),deltaTheta/(0.5*PI)+0.5,0.0,1.0);
	//outColor = vec4(abs(phi)/(PI),0.0,0.0,1.0);
	//outColor = vec4(theta/(PI),0.0,0.0,1.0);
}