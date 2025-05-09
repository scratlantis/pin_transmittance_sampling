#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(binding = 0) uniform VIEW {View view;};
layout(location = 0) in vec3 vertex_position;

layout(location = 1) in mat4 modelMat;
layout(location = 5) in mat4 invModelMat;

//layout(location = 1) out vec3 fragment_direction;
layout(location = 0) out vec3 fragment_position;
layout(location = 1) flat out int fragment_instanceId;
layout(location = 2) out mat4 fragment_modelMat;
layout(location = 6) out mat4 fragment_invModelMat;







void main()
{
	vec4 pos = vec4( vertex_position,1.0);
	vec4 worldPos = modelMat * pos;

	mat4 secondaryViewMat = mat4(-1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,-1.0,0.0, 0.0,0.0,-10.0,1.0);
	vec4 viewPos = secondaryViewMat*worldPos;
	vec4 clipPos = (view.secondaryProjectionMat * viewPos);
	fragment_position = pos.xyz;
	fragment_instanceId = gl_InstanceIndex;
	fragment_modelMat = modelMat;
	fragment_invModelMat = invModelMat;
	gl_Position =  vec4(clipPos);
}