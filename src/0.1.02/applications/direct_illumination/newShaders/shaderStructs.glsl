// Included in executable

struct CamConst
{
	mat4 projectionMat;
	mat4 viewMat;
	mat4 inverseViewMat;
	mat4 inverseProjectionMat;
	vec4 camPos;
	vec4 camFixpoint;
};

struct ViewConst
{
	uint width;
	uint height;
	uint frameIdx;
	uint padding[1];
};

struct GuiVar
{
	uint useEnvMap;
	float secRayLength;
	float positionalJitter;
	float angularJitter;

	uint showPins;
	float pinSelectionCoef;
	float gaussianWeight;
	float stdDeviation;
	//uint padding[1];
};



#ifndef CCP_IMPLEMENTATION
struct Transform
{
	mat4 mat;
	mat4 invMat;
};
struct Gaussian
{
	vec3  mean;
	float variance;
};

struct Pin
{
	vec2 theta;
	vec2 phi;
};


struct PinData
{
	uint pinIndex;
	uint padding[3];
};

struct PinGridEntry
{
	Pin pin;
	PinData data;
};
#endif







