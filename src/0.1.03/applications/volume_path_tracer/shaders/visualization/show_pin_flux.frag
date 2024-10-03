
#version 460
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in float inU;

void main()
{
	vec4 color = (0.5 - ( abs(inU-0.5) )) * vec4(2.0);
	color.gb *= color.gb;
	outColor = color;

	//outColor.a = (0.5 - ( abs(inU-0.5) ))*2.0;
	//outColor.rgb = vec3(1.0);

}