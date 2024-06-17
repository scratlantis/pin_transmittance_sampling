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