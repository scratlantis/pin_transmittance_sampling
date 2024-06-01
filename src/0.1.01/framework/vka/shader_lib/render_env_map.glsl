#pragma once


#version 460
#extension GL_GOOGLE_include_directive : enable
#include "pins_common.glsl"

layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;
layout(binding = 0) uniform sampler envMapSampler;
layout(binding = 1) uniform texture2D envMap;
layout(binding = 2, rgba8) uniform image2D imageOut;
void main()
{
	uvec2 gID = gl_GlobalInvocationID.xy;
	if(gID.x >= view.width || gID.y >= view.height) return;
	ivec2 p = ivec2(gID);
	outColor = vec4(0.0,0.0,0.0,0.0);
	imageStore(imageOut, p, outColor);
}
}
