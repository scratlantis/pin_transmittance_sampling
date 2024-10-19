#define PLOT_FLAG_ACCUMULATE 1

#define PLOT_DATA_TYPE_FLOAT 0
#define PLOT_DATA_TYPE_UINT 1
struct GLSLYListPlot
{
// general fields
	uint count;
	uint offset;
	float stride;
	uint invocationID;

	uint plotID;
	uint flags;
	uint dataType;
// special fields
	uint accumulationCount;
	//uint padding[1];
};

