
#ifndef SHADER_CONST_H
#define SHADER_CONST_H


#define SHADER_CONST_BINDING_COUNT 3
layout(binding = SHADER_CONST_OFFSET) uniform FRAME									 
{																		 
	GLSLFrame frame;													 
};																		 
layout(binding = SHADER_CONST_OFFSET + 1) uniform VIEW								 
{																		 
	GLSLView view;													     
};																		 
layout(binding = SHADER_CONST_OFFSET + 2) uniform PARAMS								 
{																		 
	GLSLParams params;													 
};

layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;




#define COMPUTE_SHADER_CHECK_ID_FRAME									 \
ivec2 gID = ivec2(gl_GlobalInvocationID.xy);							 \
if(gID.x >= frame.width || gID.y >= frame.height){return;}


#endif