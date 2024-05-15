#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(location = 0) in vec3 fragment_position;
layout(location = 1) flat in int fragment_pin_ID;
layout(location = 2) in mat4 fragment_modelMat;
layout(location = 6) in mat4 fragment_invModelMat;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform VIEW {View view;};
layout(binding = 1) buffer PINS_USED {uint pinsUsed[PIN_COUNT];};


void main()
{

	if(view.showPins == 0)
	{
		discard;
	}
	if(pinsUsed[fragment_pin_ID] == 0)
	{
		if(view.showPins == 1)
		{
			discard;
		}
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
	}


	//if(pinsUsed[fragment_pin_ID] == 1)
	//{
	//	outColor = vec4(0.0, 1.0, 0.0, 1.0);
	//}
}