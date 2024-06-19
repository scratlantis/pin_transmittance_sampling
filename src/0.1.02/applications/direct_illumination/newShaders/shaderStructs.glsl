// Included in executable
struct FrameConstants // 16 Byte Alignment
{
	uint frameIdx;
	uint padding[3];
};

struct ViewConstants // 64 Byte Alignment
{
	mat4 projectionMat;
	mat4 viewMat;
	mat4 inverseViewMat;
	mat4 inverseProjectionMat;

	vec4 camPos;
	vec4 camFixpoint;
	uint width;
	uint height;
	uint padding[6];
};

struct GuiVar // 16 Byte Alignment
{
	uint useEnvMap;
	float secRayLength;
	float positionalJitter;
	float angularJitter;

	uint showPins;
	float pinSelectionCoef;
	uint padding[2];
};

struct Volume // 64 Byte Alignment
{
	mat4 modelMat;
	mat4 inverseModelMat;
};

#ifndef CCP_IMPLEMENTATION
struct Transform // 
{
	mat4 mat;
	mat4 invMat;
};
#endif

struct ShaderConst
{
	ViewConstants view;
	Transform volume;
	FrameConstants frame;
	GuiVar gui;
};

