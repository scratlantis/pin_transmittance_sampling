#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/core/core_common.h>
#include "../HdrImagePdfCache.h"

namespace vka
{
namespace pbr
{

// Struct needs to be evaluated inside the shader
struct OffsetBufferEntry
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t materialOffset;
	uint32_t padding;
};
static_assert(sizeof(OffsetBufferEntry) == 16, "Size is not correct");

template <class Material>
struct material_type;

class USceneData
{
  public:
	Buffer             vertexBuffer;
	Buffer             indexBuffer;
	Buffer             offsetBuffer;
	Buffer             materialBuffer;
	Buffer             areaLightBuffer;
	TLAS               tlas;

	std::vector<Image> textures;
	Image              envMap;
	Buffer             envMapPdfBuffer;

	void garbageCollect()
	{
		vertexBuffer->garbageCollect();
		indexBuffer->garbageCollect();
		offsetBuffer->garbageCollect();
		materialBuffer->garbageCollect();
		areaLightBuffer->garbageCollect();
		tlas->garbageCollect();
	}

	// build tlas
	void build(CmdBuffer cmdBuf, Buffer instanceBuffer);
};



class USceneBuilderBase
{
  protected:
	// Model related data
	std::unordered_map<std::string, uint32_t> textureIndexMap;
	std::vector<ModelData>                    modelList;
	std::vector<glm::mat4>                    transformList;
	std::unordered_map<std::string, uint32_t> indexMap;


	// Ohter data
	std::string envMapName;

	HdrImagePdfCache* pdfCache;

	// create material buffer
	virtual void loadMaterials(CmdBuffer cmdBuf, Buffer buffer) = 0;

  public:
	
	USceneBuilderBase(HdrImagePdfCache* pdfCache) : pdfCache(pdfCache) {}

	void loadEnvMap(const ImagePdfKey &key);

	// load model
	void addModel(CmdBuffer cmdBuf, std::string path, glm::mat4 transform = glm::mat4(1.0), uint32_t loadFlags = MODEL_LOAD_FLAG_IS_OPAQUE);

	virtual void addModelInternal(CmdBuffer cmdBuf, ModelCache *pModelCache, std::string path, uint32_t loadFlags) = 0;

	// copy together buffers, create tlas
	USceneData create(CmdBuffer cmdBuf, IResourcePool *pPool);

	// crate and upload instance buffer
	Buffer uploadInstanceData(CmdBuffer cmdBuf, IResourcePool *pPool);

	// set transform for model
	void setTransform(std::string path, glm::mat4 transform);

	// revert to initial state
	void reset();
};

template <class Vertex, class Material>
class USceneBuilder : public USceneBuilderBase
{

	// create material buffer
	virtual void loadMaterials(CmdBuffer cmdBuf, Buffer buffer) override
	{
		std::vector<Material> materialList;
		for (auto& model : modelList)
		{
			for (auto& mtl : model.mtl)
			{
				materialList.push_back(material_type<Material>().load_mtl(mtl, textureIndexMap));
			}
		}
		cmdWriteCopy(cmdBuf, buffer, materialList.data(), materialList.size() * sizeof(Material));
	}
	void addModelInternal(CmdBuffer cmdBuf, ModelCache *pModelCache, std::string path, uint32_t loadFlags) override
	{
		modelList.push_back(pModelCache->fetch<Vertex>(cmdBuf, path, loadFlags));
	}

  public:

	USceneBuilder(HdrImagePdfCache *pdfCache) :
	    USceneBuilderBase(pdfCache)
	{}
	// load model

};

}        // namespace pbr
}        // namespace vka
