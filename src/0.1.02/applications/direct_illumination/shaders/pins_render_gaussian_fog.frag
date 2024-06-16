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
layout(binding = 1) buffer GAUSSIANS {Gaussian gaussians[GAUSSIAN_COUNT];};
//layout(binding = 2) uniform sampler wireFrameSampler;
//layout(binding = 3) uniform texture2D wireFrameColorImg;
//layout(binding = 4) uniform texture2D wireFramePosImg;

void main()
{
	vec3 worldPos = (fragment_modelMat*vec4(fragment_position,1.0)).xyz;
	vec3 dirWorldSpace = normalize(worldPos-view.camPos.xyz);
	vec3 dir = (fragment_invModelMat * vec4(dirWorldSpace,0.0)).xyz;
	dir = normalize(dir);

	ivec2 p = ivec2(gl_FragCoord.xy);
	vec4 wireFrameColor = vec4(0.0);//texelFetch(sampler2D(wireFrameColorImg, wireFrameSampler), p, 0);
	vec4 wireFramePos = vec4(0.0); //texelFetch(sampler2D(wireFramePosImg, wireFrameSampler), p, 0);

	assureNotZero(dir);
	vec3 invDir = 1.0 / dir;
	vec3 c1 = -fragment_position*invDir;
	vec3 c2 = c1 + invDir;
	float tMin = max(max(min(c1.x, c2.x), min(c1.y, c2.y)), min(c1.z, c2.z));
	float tMax = min(min(max(c1.x, c2.x), max(c1.y, c2.y)), max(c1.z, c2.z));
	vec3 entryPoint = fragment_position + tMin*dir;
	vec3 exitPoint = fragment_position + tMax*dir;

	vec3 entryPointWorld = (fragment_modelMat * vec4(entryPoint,1.0)).xyz;
	vec3 exitPointWorld = (fragment_modelMat * vec4(exitPoint,1.0)).xyz;
	vec3 wireFramePosLocal = ( fragment_invModelMat * vec4(wireFramePos.xyz,1.0)).xyz;

	bool isBefore = false;

	if(wireFramePos != vec4(0))
	{
		if(distance(entryPointWorld, view.camPos.xyz) > distance(wireFramePos.xyz, view.camPos.xyz))
		{
			isBefore = true;
		}
		else if(distance(exitPointWorld, view.camPos.xyz) > distance(wireFramePos.xyz, view.camPos.xyz))
		{
			exitPoint = wireFramePosLocal;
		}
	}

	float transmittance = 1.0;
	float weight = clamp(10.0/float(GAUSSIAN_COUNT), 0.0,1.0);
	for(int i = 0; i < GAUSSIAN_COUNT; i++)
	{
		transmittance *= clamp(1.0-weight*evalTransmittanceGaussianSegment(entryPoint, exitPoint, gaussians[i]), 0.0, 1.0);
	}

	outColor = vec4(vec3(transmittance), 1.0);
	outColor.xyz = transmittance*wireFrameColor.xyz;

	if(isBefore)
	{
		transmittance = 1.0;
	}

	if(wireFramePos != vec4(0))
	{
		vec3 wireFramePosLocal = ( fragment_invModelMat * vec4(wireFramePos.xyz,1.0)).xyz;
		outColor.xyz = transmittance*wireFrameColor.xyz*4.0;
	}
	outColor = vec4(0.0,0.0,1.0,1.0);
}