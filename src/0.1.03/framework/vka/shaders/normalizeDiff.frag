#version 460
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC
{
	float x;
	float y;
	float width;
	float height;
} pc;

layout(binding = 0) uniform sampler2D smpA;
layout(binding = 1) uniform sampler2D smpB;

void main()
{
	vec4 colorA = texture(smpA,inUV*vec2(pc.width, pc.height)+vec2(pc.x,pc.y));
	if(colorA.a == 0.0)
	{
		colorA.xyz = vec3(0.0);
	}
	else
	{
		colorA.xyz = colorA.xyz / colorA.a;
	}

	vec4 colorB = texture(smpB,inUV*vec2(pc.width, pc.height)+vec2(pc.x,pc.y));
	if(colorB.a == 0.0)
	{
		colorB.xyz = vec3(0.0);
	}
	else
	{
		colorB.xyz = colorB.xyz / colorB.a;
	}

	outColor.a = 1.0;
	outColor.rgb = vec3(0.5 + -(colorA.r - colorB.r + colorA.g - colorB.g + colorA.b - colorB.b));
}