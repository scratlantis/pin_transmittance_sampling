#version 460
#extension GL_GOOGLE_include_directive : enable
#include "shaderStructs.glsl"
layout(binding = 0) uniform CAM_CONST {CamConst uCam;};

layout(location = 0) in vec3 vs_position;
layout(location = 1) in vec3 vs_normal;

layout(location = 2) in mat4 modelMat;
layout(location = 6) in mat4 invModelMat;

layout(location = 0) out vec3 fs_world_pos;
layout(location = 1) out vec3 fs_world_normal;


void main()
{
	vec4 pos = vec4( vs_position,1.0);
	vec4 normal = vec4( normalize(vs_normal),0.0);

	vec4 worldPos = modelMat * pos;
	vec4 worldNormal = modelMat * normal;

	fs_world_pos = worldPos.xyz;
	fs_world_normal = vs_normal.xyz;

	vec4 viewPos = uCam.viewMat*worldPos;
	vec4 clipPos = uCam.projectionMat * viewPos;
	gl_Position =  vec4(clipPos);
}