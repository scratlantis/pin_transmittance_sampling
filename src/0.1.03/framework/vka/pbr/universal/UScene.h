#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/core/core_common.h>

namespace vka
{
namespace pbr
{

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
		envMapPdfBuffer->garbageCollect();
	}

	// build tlas
	void build(Buffer instanceBuffer);
};

class USceneBuilderBase
{
  protected:
	std::unordered_map<std::string, uint32_t> textureIndexMap;
	std::vector<std::string>                  textureNames;
	std::vector<ModelData>                    modelList;
	std::vector<glm::mat4>                    transformList;
	std::unordered_map<std::string, uint32_t> indexMap;

	// create material buffer
	virtual Buffer createMaterialBuffer() = 0;

  public:
	// load model
	virtual void addModel(CmdBuffer cmdBuf, std::string path, glm::mat4 transform = glm::mat4(1.0), uint32_t loadFlags = MODEL_LOAD_FLAG_IS_OPAQUE) = 0;

	// copy together buffers, load textures, load envmap, create tlas, create blas, build blas, create pdf buffers
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
	virtual Buffer createMaterialBuffer()
	{
		std::vector<Material> materialList;
		for (auto& model : modelList)
		{
			for (auto& mtl : model.mtl)
			{
				materialList.push_back(material_type<Material>().load_mtl(mtl, textureIndexMap, textureNames));
			}
		}
		return nullptr;        // todo
	}

  public:
	// load model
	void addModel(CmdBuffer cmdBuf, std::string path, glm::mat4 transform = glm::mat4(1.0), uint32_t loadFlags = MODEL_LOAD_FLAG_IS_OPAQUE)
	{
		modelList.push_back(gState.modelCache->fetch<Vertex>(cmdBuf, path, loadFlags));
	}

};

}        // namespace pbr
}        // namespace vka
