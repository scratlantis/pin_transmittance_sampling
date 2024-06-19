#pragma once
#include <glm/glm.hpp>

struct Pin
{
	glm::vec2 theta;
	glm::vec2 phi;
};

struct PinData
{
	uint32_t pinIndex;
	uint32_t padding[3];
};
struct PinGridEntry
{
	Pin     pin;
	PinData data;
};

struct Cube
{
	glm::mat4 modelMat;

	glm::mat4 invModelMatrix;
};

struct View
{
	glm::mat4 viewMat;
	glm::mat4 inverseViewMat;
	glm::mat4 projectionMat;
	glm::mat4 inverseProjectionMat;
	glm::mat4 rotationMatrix;

	glm::vec4 camPos;
	uint32_t  width;
	uint32_t  height;
	uint32_t  frameCounter;
	uint32_t  showPins;

	float    pinSelectionCoef;
	float    expMovingAverageCoef;
	uint32_t secondaryWidth;
	uint32_t secondaryHeight;

	glm::mat4 secondaryProjectionMat;
	glm::mat4 secondaryInverseProjectionMat;
	glm::vec4 probe;

	glm::mat4 fogModelMatrix;
	glm::mat4 fogInvModelMatrix;

	float    secRayLength;
	float    positionalJitter;
	float    angularJitter;
	uint32_t useEnvMap;

	Cube cube;
};

struct Gaussian
{
	glm::vec3 mean;
	float     variance;
};

class PosVertex
{
  public:
	PosVertex(){};
	PosVertex(glm::vec3 pos) :
	    pos(pos)
	{}
	glm::vec3                              pos;


	static VertexDataLayout getVertexDataLayout()
	{
		VertexDataLayout layout{};
		layout.formats = {VK_FORMAT_R32G32B32_SFLOAT};
		layout.offsets = {offsetof(PosVertex, pos)};
		layout.stride = sizeof(PosVertex);
		return layout;
	}

	static void parse(void *vertexPointer, uint32_t idx, const tinyobj::attrib_t &vertexAttributes)
	{
		glm::vec3 *vertex = (glm::vec3 *) vertexPointer;
		vertex->x         = vertexAttributes.vertices[idx * 3];
		vertex->y         = vertexAttributes.vertices[idx * 3 + 1];
		vertex->z         = vertexAttributes.vertices[idx * 3 + 2];
	}

	static void parseObj(VkaBuffer vertexBuffer, const std::vector<ObjVertex> &vertexList, VkaBuffer indexBuffer, const std::vector<Index> &indexList)
	{
		PosVertex *vertexData = static_cast<PosVertex *>(vkaMapStageing(vertexBuffer, vertexList.size() * sizeof(PosVertex)));
		for (size_t i = 0; i < vertexList.size(); i++)
		{
			vertexData[i].pos    = vertexList[i].v;
		}
		vkaUnmap(vertexBuffer);
		vkaWriteStaging(indexBuffer, static_cast<const void *>(indexList.data()), indexList.size() * sizeof(Index));
	}
};

class PosNormalVertex
{
  public:
	PosNormalVertex(){};
	PosNormalVertex(glm::vec3 pos, glm::vec3 vertex) :
	    pos(pos)
	{}
	glm::vec3 pos;
	glm::vec3 normal;

	static VertexDataLayout getVertexDataLayout()
	{
		VertexDataLayout layout{};
		layout.formats = {VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT};
		layout.offsets = {
		    offsetof(PosNormalVertex, pos),
		    offsetof(PosNormalVertex, normal),
		};
		layout.stride = sizeof(PosNormalVertex);
		return layout;
	}

	static void parse(void *vertexPointer, uint32_t idx, const tinyobj::attrib_t &vertexAttributes)
	{
		PosNormalVertex *vertex = (PosNormalVertex *) vertexPointer;
		vertex->pos.x         = vertexAttributes.vertices[idx * 3];
		vertex->pos.y         = vertexAttributes.vertices[idx * 3 + 1];
		vertex->pos.z         = vertexAttributes.vertices[idx * 3 + 2];

		vertex->normal.x = vertexAttributes.normals[idx * 3];
		vertex->normal.y = vertexAttributes.normals[idx * 3 + 1];
		vertex->normal.z = vertexAttributes.normals[idx * 3 + 2];
	}

	static void parseObj(VkaBuffer vertexBuffer, const std::vector<ObjVertex> &vertexList, VkaBuffer indexBuffer, const std::vector<Index> &indexList)
	{
		PosNormalVertex *vertexData = static_cast<PosNormalVertex *>(vkaMapStageing(vertexBuffer, vertexList.size() * sizeof(PosNormalVertex)));
		for (size_t i = 0; i < vertexList.size(); i++)
		{
			vertexData[i].pos = vertexList[i].v;
			vertexData[i].normal = vertexList[i].vn;
		}
		vkaUnmap(vertexBuffer);
		vkaWriteStaging(indexBuffer, static_cast<const void *>(indexList.data()), indexList.size() * sizeof(Index));

	}
};

class Transform
{
  public:
	glm::mat4 mat;
	glm::mat4 invMat;
	Transform(glm::mat4 mat = glm::mat4(1.0)) :
	    mat(mat), invMat(glm::inverse(mat))
	{}

	static VertexDataLayout getVertexDataLayout()
	{
		VertexDataLayout layout{};
		layout.formats =
		{
		        VK_FORMAT_R32G32B32A32_SFLOAT,
		        VK_FORMAT_R32G32B32A32_SFLOAT,
		        VK_FORMAT_R32G32B32A32_SFLOAT,
		        VK_FORMAT_R32G32B32A32_SFLOAT,
		        VK_FORMAT_R32G32B32A32_SFLOAT,
		        VK_FORMAT_R32G32B32A32_SFLOAT,
		        VK_FORMAT_R32G32B32A32_SFLOAT,
		        VK_FORMAT_R32G32B32A32_SFLOAT
		};
		layout.offsets =
		{
			0,
		     sizeof(glm::vec4),
		     2 * sizeof(glm::vec4),
		     3 * sizeof(glm::vec4),
		     4 * sizeof(glm::vec4),
		     5 * sizeof(glm::vec4),
		     6 * sizeof(glm::vec4),
		     7 * sizeof(glm::vec4)
		};
		layout.stride = sizeof(Transform);
		return layout;
	}
};

typedef uint32_t uint;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;


#define CCP_IMPLEMENTATION
#include "newShaders/shaderStructs.glsl"


//struct FrameConstants
//{
//	uint frameIdx;
//	uint padding[3];
//};
//
//struct ViewConstants
//{
//	mat4 viewMat;
//	mat4 inverseViewMat;
//	mat4 projectionMat;
//	mat4 inverseProjectionMat;
//
//	vec4 camPos;
//
//	uint width;
//	uint height;
//	uint padding[2];
//};
//
//struct GuiVar
//{
//	uint padding[4];
//};
//
//struct Volume
//{
//	mat4 modelMat;
//	mat4 inverseModelMat;
//};
//
//struct ShaderConst
//{
//	FrameConstants frame;
//	ViewConstants  view;
//	GuiVar         gui;
//	Volume         volume;
//};


// new

