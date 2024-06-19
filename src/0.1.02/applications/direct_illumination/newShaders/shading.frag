#version 460
#extension GL_GOOGLE_include_directive : enable
#include "shaderStructs.glsl"
layout(location = 0) in vec3 fs_world_pos;
layout(location = 1) in vec3 fs_world_normal;
layout(location = 0) out vec4 outColor;

void main()
{
	outColor.a = 1.0;
	outColor.rgb = (normalize(fs_world_normal));
}
