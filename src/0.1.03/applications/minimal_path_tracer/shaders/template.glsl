
#ifndef FORMAT1
#define FORMAT1 rgba32f
#endif

#define SHADER_PRELUDE1													 \
layout(binding = 0) uniform FRAME										 \
{																		 \
	GLSLFrame frame;														 \
};																		 \
layout(binding = 1) uniform VIEW										 \
{																		 \
	GLSLView view;															 \
};																		 \
layout(binding = 2) uniform PARAMS										 \
{																		 \
	GLSLParams params;												 \
};

#define SHADER_PRELUDE2													 \
layout(binding = 0) uniform FRAME										 \
{																		 \
	Frame frame;														 \
};																		 \
layout(binding = 2) uniform PARAMS										 \
{																		 \
	GlobalParams params;												 \
};																		 