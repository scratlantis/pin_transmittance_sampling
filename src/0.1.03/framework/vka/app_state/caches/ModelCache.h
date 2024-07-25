#pragma once
#include <vka/core/stateless/utility/macros.h>
#include <vka/core/resources/unique/Buffer.h>
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
	Buffer_R *vertexBuffer;
	Buffer_R *indexBuffer;
	uint32_t  offset;
	uint32_t  count;
};

struct ModelData
{
	Buffer_R             *vertexBuffer;
	Buffer_R             *indexBuffer;
	std::vector<uint32_t> indexOffsets;

	bool operator==(const ModelData &other) const
	{
		return vertexBuffer == other.vertexBuffer && indexBuffer == other.indexBuffer && cmpVector(indexOffsets, other.indexOffsets);
	};
	vka::hash_t hash() const
	{
		return vertexBuffer HASHC indexBuffer HASHC hashVector(indexOffsets);
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

  public:
	ModelCache(IResourcePool *pPool, std::string modelPath) :
	    modelPath(modelPath), pPool(pPool)
	{}
	void      clear();
	//ModelData fetch(VkaCommandBuffer cmdBuf, std::string path, void (*parse)(VkaBuffer vertexBuffer, const std::vector<ObjVertex> &vertexList, VkaBuffer indexBuffer, const std::vector<Index> &indexList));
};
}        // namespace vka