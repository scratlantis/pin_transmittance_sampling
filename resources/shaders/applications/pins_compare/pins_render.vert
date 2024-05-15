#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(binding = 0) uniform VIEW {View view;};
layout(location = 0) in vec3 vertex_position;

layout(location = 1) in mat4 modelMat;
layout(location = 5) in mat4 invModelMat;

layout(location = 0) out vec3 fragment_position;
layout(location = 1) out vec3 fragment_color;


void main()
{
	//fragment_position = (view.projectionMat * view.viewMat * modelMat * vec4(vertex_position,1.0)).xyz;
	//fragment_position = (view.projectionMat * view.viewMat * vec4(vertex_position,1.0)).xyz;

	vec4 pos = vec4( vec3(1.0) + vertex_position.xzy * 0.15,1.0);
	fragment_position = (view.viewMat * pos).xyz;
	fragment_position = (view.projectionMat * view.viewMat * pos).xyz;
	vec4 viewPos = (view.projectionMat * view.viewMat * pos);
	fragment_position = viewPos.xyz / viewPos.w;
	//fragment_position = ( view.viewMat * pos).xyz;

	//fragment_position.x *= -1.0;
	//fragment_position = pos.xyz;
	fragment_color.x = pos.z*0.8;
	fragment_color.y = 1.0;

	gl_Position = vec4(0.1*vertex_position.xy,0.0,1.0);
	gl_Position = vec4(fragment_position,1.0);
}