#pragma once
#include <unordered_map>
#include <vka/core/common.h>
#include <vka/interface/common.h>
#include "tiny_obj_loader.h"
namespace vka
{


static void parse_VEC3(void *vertexPointer, uint32_t idx, const tinyobj::attrib_t &vertexAttributes)
{
	glm::vec3 *vertex = (glm::vec3 *) vertexPointer;
	vertex->x         = vertexAttributes.vertices[idx * 3];
	vertex->y         = vertexAttributes.vertices[idx * 3 + 1];
	vertex->z         = vertexAttributes.vertices[idx * 3 + 2];
}

class ModelCache
{
	std::unordered_map<std::string, ModelData> map;
	std::string                                modelPath;
	IResourcePool                        *pPool;
  public:
	ModelCache(IResourcePool *pPool, std::string modelPath) :
	    modelPath(modelPath), pPool(pPool)
	{}
	void      clear();
	ModelData fetch(VkaCommandBuffer cmdBuf, std::string path, uint32_t bytesPerVertex, void (*parse)(void *vertexPointer, uint32_t idx, const tinyobj::attrib_t &vertexAttributes));

};
}

DECLARE_HASH(ModelData, hash);