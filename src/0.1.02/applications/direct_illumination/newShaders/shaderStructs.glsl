// Included in executable
struct FrameConstants
{
	uint frameIdx;
	uint padding[3];
};

struct ViewConstants
{
	mat4 viewMat;
	mat4 inverseViewMat;
	mat4 projectionMat;
	mat4 inverseProjectionMat;

	vec4 camPos;
	vec4 camFixpoint;

	uint width;
	uint height;
	uint padding[2];
};

struct GuiVar
{
	uint useEnvMap;
	float secRayLength;
	float positionalJitter;
	float angularJitter;

	uint showPins;
	float pinSelectionCoef;
	uint padding[2];
};

struct Volume
{
	mat4 modelMat;
	mat4 inverseModelMat;
};


struct ShaderConst
{
	FrameConstants frame;
	ViewConstants view;
	GuiVar gui;
	Volume volume;
};

#ifndef CCP_IMPLEMENTATION
struct Transform
{
	mat4 mat;
	mat4 invMat;
};
#endif
