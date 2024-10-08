#version 460
#extension GL_GOOGLE_include_directive : enable

layout(binding = 0) uniform VP{mat4 vp;};
layout(location = 0) in vec3 pos;
layout(binding = 1) readonly buffer PIN_FLUX {float pinFlux[];};

layout (location = 0) out float outFlux;

void main() 
{
	gl_Position = vp * vec4(pos, 1.0);
	outFlux = pinFlux[gl_VertexIndex/2];
}