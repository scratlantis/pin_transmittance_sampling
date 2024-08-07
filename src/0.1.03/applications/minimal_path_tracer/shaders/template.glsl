
#ifndef FORMAT1
#define FORMAT1 rgba32f
#endif

#define SHADER_BLOCK_FRAME_VIEW_PARAMS(OFFSET)							 \
layout(binding = OFFSET) uniform FRAME									 \
{																		 \
	GLSLFrame frame;													 \
};																		 \
layout(binding = OFFSET + 1) uniform VIEW								 \
{																		 \
	GLSLView view;													     \
};																		 \
layout(binding = OFFSET + 2) uniform PARAMS								 \
{																		 \
	GLSLParams params;													 \
};

#define SHADER_BLOCK_FRAME_PARAMS										 \
layout(binding = OFFSET) uniform FRAME									 \
{																		 \
	Frame frame;														 \
};																		 \
layout(binding = OFFSET + 1) uniform PARAMS								 \
{																		 \
	GlobalParams params;												 \
};

#define COMPUTE_SHADER_CHECK_ID_FRAME									 \
ivec2 gID = ivec2(gl_GlobalInvocationID.xy);							 \
if(gID.x >= frame.width || gID.y >= frame.height){return;}

#define COMPUTE_SHADER_ID_IN layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;