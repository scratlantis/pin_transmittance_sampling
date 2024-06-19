#version 460
#extension GL_GOOGLE_include_directive : enable
#include "shaderStructs.glsl"
#include "../shaders/pins_common.glsl"
layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) uniform SHADER_CONST {ShaderConst c;};
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in mat4 modelMat;
layout(location = 5) in mat4 invModelMat;

//layout(location = 0) out vec3 fragment_position;
//layout(location = 1) flat out int fragment_instanceId;
//layout(location = 2) out mat4 fragment_modelMat;
//layout(location = 6) out mat4 fragment_invModelMat;


void main()
{
	vec4 pos = vec4( vertex_position,1.0);


	vec4 worldPos = modelMat * pos;
	//vec4 worldPos = modelMat * pos;
	
	//vec4 viewPos = view.viewMat * worldPos;
	vec4 viewPos = c.view.viewMat*worldPos;

	//vec4 clipPos = (view.projectionMat * viewPos);
	vec4 clipPos = c.view.projectionMat * viewPos;

	gl_Position =  vec4(clipPos);
}