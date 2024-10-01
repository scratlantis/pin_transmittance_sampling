#version 460
#extension GL_GOOGLE_include_directive : enable

#include "pin_state_flags.glsl"


layout(location = 0) flat in uint state;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D smp;

struct PushStruct
{
	float   alpha_0;
	float   alpha_1;
};
layout(push_constant) uniform PC {PushStruct pc;};


void main()
{
#ifdef OPAQUE
	if((state & PIN_STATE_FLAGS_CELL_SELECTED) != 0)
	{
		outColor = vec4(0.0, 0.0, 1.0, 1.0);

		if((state & PIN_STATE_FLAGS_PIN_SELECTED) != 0)
		{
			outColor = vec4(1.0, 1.0, 1.0, 1.0);
		}
	}
	else
	{
		discard; // discard transparent
	}
#else
	if(state == 0)
	{
		outColor = vec4(1.0, 0.0, 0.0, pc.alpha_0);
	}
	else if((state == PIN_STATE_FLAGS_INSIDE_GRID))
	{
		outColor = vec4(0.0, 1.0, 0.0, pc.alpha_1);
	}
	else
	{
		discard; // discard opaque
	}
#endif
}