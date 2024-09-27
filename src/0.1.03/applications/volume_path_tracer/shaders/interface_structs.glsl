// Must be cpp compatible
#ifndef INTERFACE_STRUCTS_H
#define INTERFACE_STRUCTS_H

struct GLSLFrame
{
	mat4 projection;
	mat4 invProjection;
	uint width;
	uint height;
	uint idx;
	uint padding[1];
};

struct GLSLView
{
	mat4 mat;
	mat4 invMat;
	vec4 pos;
};


#define BUTTON_RELEASED 0
#define BUTTON_PRESSED 1
#define BUTTON_PRESS_EVENT 2
#define BUTTON_RELEASE_EVENT 3

struct GLSLParams
{
	vec2 cursorPos;
	uint leftMB;
	uint controlKEY;

	float pinSampleLocation;
	uint pathSampling;
	uint jitterPinSampleLocation;
	float jitterPinSamplePos;
};


struct GLSLVertex
{
	vec3 pos;
	uint padding_0[1];

	vec3 normal;
	uint padding_1[1];

	vec2 uv;
	uint padding_2[2];
};

struct GLSLMaterial
{
	vec3 albedo;
	uint padding_0[1];

	vec3 specular;
	uint padding_1[1];

	vec3 emission;
	uint padding_2[1];

	float roughness;
	float f0; //  0.16 reflectance^2
	uint padding_3[2];
};

#define TYPE_DEFAUL 0
#define TYPE_LINE_SEGMENT 1
struct GLSLInstance
{
	mat4 mat;

	vec3 color;
	uint cullMask;

	uint type;
	uint padding[3];
};
#define LINE_SEGMENTS_PER_BOUNCE 5
struct GLSLLineSegment
{
	vec3 start;
	uint padding1[1];

	vec3 end;
	uint padding2[1];

	vec3 color;
	uint cullMask;
};

struct GLSLMediumInstance
{
	mat4 mat;
	mat4 invMat;
	vec3 albedo;
	uint padding[1];
};

// use vec2 so struct can be alligned to 8 bytes
struct GLSLPin
{
	vec2 ax_ay;
	vec2 az_bx;
	vec2 by_bz;
};

struct GLSLPinGridEntry
{
	GLSLPin pin; // 24 bytes
	uint idx; // 4 bytes
	uint padding[1]; // 4 bytes
};

struct GLSLAccelerationStructureInstanceKHR
{
    float	transform[12];
    uint    instanceCustomIndex24_mask8;
    uint	instanceShaderBindingTableRecordOffset24_flags8;
    uint    asRef1;
    uint    asRef2;
};

#endif // INTERFACE_H
