// Must be cpp compatible
#ifndef INTERFACE_H
#define INTERFACE_H

struct GLSLFrame
{
	uint width;
	uint height;
	uint idx;
	uint padding[1];
	mat4 projection;
	mat4 invProjection;
};

struct GLSLView
{
	mat4 mat;
	mat4 invMat;
	vec4 pos;
};

struct GLSLParams
{
	uint padding[4];
};

#endif // INTERFACE_H
