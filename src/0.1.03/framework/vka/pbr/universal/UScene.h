#pragma once
#include <vka/core/core_common.h>
#include <vka/advanced_state/AdvancedState.h>


namespace vka
{
	namespace pbr
	{

	template <class Material>
	struct material_type
	{
		Material loadMtl(WavefrontMaterial mtl, std::unordered_map<std::string, uint32_t > & textureIndexMap, std::vector<std::string>& textureNames)
		{
		    return Material::loadMtl(mtl, textureIndexMap, textureNames);
		}
	};

	template <class Vertex, class Material>
    class USceneData
    {
	  public:
	    material_type<Material> materialType;        // Common material type for all geometry
	    vertex_type<Vertex>     vertexType;          // Common vertex type for all geometry
	    Buffer                  vertexBuffer;
	    Buffer                  indexBuffer;
	    Buffer                  offsetBuffer;
	    Buffer                  materialBuffer;
	    Buffer                  areaLightBuffer;
	    TLAS                    tlas;
	    std::vector<Image>      textures;
		Image                   envMap;
		Buffer                  envMapPdfBuffer;
    };

	template <class Vertex, class Material>
	class USceneBuilder
	{
	    std::unordered_map<std::string, uint32_t > textureIndexMap;
	    std::vector<std::string> textureNames;
	    IResourcePool           *pPool;
	    std::vector<ModelData>   modelList;
	};

	}
}
