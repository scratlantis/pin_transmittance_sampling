#pragma once
#include <vka/core/stateless/utility/macros.h>
#include <vka/core/resources/unique/Buffer.h>
#include <vka/core/core_utility/types.h>
#include <unordered_map>
namespace vka
{
typedef uint32_t Index;

struct ObjVertex
{
	glm::vec3 v;
	glm::vec2 vt;
	glm::vec3 vn;

	ObjVertex(glm::vec3 v, glm::vec2 vt, glm::vec3 vn) :
	    v(v), vt(vt), vn(vn)
	{}
	ObjVertex() :
	    v(0), vt(0), vn(0)
	{}
	bool operator==(const ObjVertex &other) const
	{
		return v == other.v && vt == other.vt && vn == other.vn;
	}
	vka::hash_t hash() const
	{
		return std::hash<glm::vec3>()(v) ^ std::hash<glm::vec2>()(vt) ^ std::hash<glm::vec3>()(vn);
	}
};

struct DrawSurface
{
	Buffer vertexBuffer;
	Buffer indexBuffer;
	uint32_t  offset;
	uint32_t  count;
};

struct ModelData
{
	Buffer                vertexBuffer;
	Buffer                indexBuffer;
	BLAS                  blas;
	std::vector<uint32_t> indexOffsets;
	uint32_t              indexCount;


	bool operator==(const ModelData &other) const
	{
		return vertexBuffer == other.vertexBuffer && indexBuffer == other.indexBuffer && cmpVector(indexOffsets, other.indexOffsets) && blas == other.blas;
	};
	vka::hash_t hash() const
	{
		return vertexBuffer HASHC indexBuffer HASHC hashVector(indexOffsets) HASHC blas;
	}

	DrawSurface getSurface(uint32_t idx) const
	{
		if (idx >= indexOffsets.size() - 1)
			return {vertexBuffer, indexBuffer, indexOffsets.back(), indexCount - indexOffsets.back()};
		else
			return {vertexBuffer, indexBuffer, indexOffsets[idx], indexOffsets[idx + 1] - indexOffsets[idx]};
	}
};

struct ModelKey
{
	std::string path;
	void       *loadFunction;

	bool operator==(const ModelKey &other) const
	{
		return path == other.path && loadFunction == loadFunction;
	}

	vka::hash_t hash() const
	{
		return std::hash<std::string>()(path) ^ std::hash<void *>()(loadFunction);
	}
};
}		// namespace vka
DECLARE_HASH(vka::ModelData, hash);
DECLARE_HASH(vka::ModelKey, hash);
DECLARE_HASH(vka::ObjVertex, hash);

namespace vka
{
class ModelCache
{
	std::unordered_map<ModelKey, ModelData> map;
	std::string                             modelPath;
	IResourcePool                          *pPool;
	VkBufferUsageFlags                      bufferUsageFlags;

  public:
	ModelCache(IResourcePool *pPool, std::string modelPath, VkBufferUsageFlags bufferUsageFlags) :
	    modelPath(modelPath), pPool(pPool), bufferUsageFlags(bufferUsageFlags)
	{}
	void      clear();
	ModelData fetch(CmdBuffer cmdBuf, std::string path, void (*parse)(Buffer vertexBuffer, const std::vector<ObjVertex> &vertexList), bool createAccelerationStructure = false, bool isOpaque = false);
};
}        // namespace vka