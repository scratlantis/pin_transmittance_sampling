#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 1) flat in int fragment_instanceId;
layout(location = 2) in mat4 fragment_modelMat;
layout(location = 6) in mat4 fragment_invModelMat;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};


void main()
{
	vec3 worldPos = (fragment_modelMat*vec4(fragment_position,1.0)).xyz;
	vec3 dirWorldSpace = normalize(worldPos-view.camPos.xyz);
	vec3 dir = (fragment_invModelMat * vec4(dirWorldSpace,0.0)).xyz;
	dir = normalize(dir);


	float transmittance = 1.0;
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		float coef = clamp(1.0-evalTransmittanceGaussian(fragment_position, dir, gaussians[i]), 0.0, 1.0);
		transmittance *= coef;
	}
	outColor = vec4(transmittance-0.1, transmittance, transmittance, 1.0);
	//outColor = vec4(dir ,1.0);
}