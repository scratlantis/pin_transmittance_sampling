#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(binding = 0) uniform VIEW {View view;};
layout(location = 0) in vec3 vertex_position;

layout(location = 1) in mat4 modelMat;
layout(location = 5) in mat4 invModelMat;

//layout(location = 1) out vec3 fragment_direction;
layout(location = 0) out vec3 fragment_position;
layout(location = 1) flat out int fragment_pin_ID;
layout(location = 2) out mat4 fragment_modelMat;
layout(location = 6) out mat4 fragment_invModelMat;







void main()
{
	vec4 pos = vec4( vertex_position,1.0);
	vec4 worldPos = modelMat * pos;
	vec4 viewPos = view.viewMat * worldPos;
	vec4 clipPos = (view.projectionMat * viewPos);
	//vec3 dir = normalize(view.camPos.xyz - worldPos.xyz);
	//fragment_direction = normalize(viewPos.xyz);
	fragment_position = pos.xyz;
	//fragment_direction = (invModelMat * vec4(dir,0.0)).xyz;
	fragment_pin_ID = gl_VertexIndex/2;
	fragment_modelMat = modelMat;
	fragment_invModelMat = invModelMat;
	gl_Position =  vec4(clipPos);
}