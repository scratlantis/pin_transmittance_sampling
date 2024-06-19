#version 460
#extension GL_GOOGLE_include_directive : enable
#include "shaderStructs.glsl"

//layout(location = 0) in vec3 fragment_position;
//layout(location = 1) flat in int fragment_instanceId;
//layout(location = 2) in mat4 fragment_modelMat;
//layout(location = 6) in mat4 fragment_invModelMat;
//layout(binding = 0) uniform SHADER_CONST {ShaderConst c;};
layout(location = 0) out vec4 outColor;

void main()
{
	outColor.a = 1.0;
	outColor.rgb = vec3(1.0, 0.0, 0.0);
}
