#include "UScene.h"

namespace vka
{
namespace pbr
{
void USceneBuilderBase::loadEnvMap(const ImagePdfKey &key)
{
	// Load env map & pdf
	// Todo
}

struct OffsetBufferEntry
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t materialOffset;
	uint32_t areaLightOffset;
};

USceneData USceneBuilderBase::create(CmdBuffer cmdBuf, IResourcePool *pPool)
    {
	    USceneData sceneData{};

		// Create buffers
		uint32_t totalVertexCount = 0;
		uint32_t totalIndexCount = 0;
	    uint32_t   vertexStride     = modelList[0].vertexLayout.stride;
		for (auto &model : modelList)
		{
		    totalVertexCount += model.vertexBuffer->getSize() / vertexStride;
		    totalIndexCount += model.indexBuffer->getSize() / sizeof(Index);
		}
	    sceneData.vertexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, totalVertexCount * vertexStride);
	    sceneData.indexBuffer  = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, totalIndexCount * sizeof(Index));

		// Copy vertex & index data
		VkDeviceSize vertexOffsett     = 0;
	    VkDeviceSize indexOffsett  = 0;
		for (auto& model : modelList)
		{
		    vertexOffsett += model.vertexBuffer->getSize() / model.vertexLayout.stride;
		    indexOffsett += model.indexBuffer->getSize() / sizeof(Index);

			cmdCopyBuffer(cmdBuf, model.vertexBuffer->getSubBuffer(), sceneData.vertexBuffer, model.vertexBuffer->getSize(), 0, vertexOffsett * vertexStride);
		}

	    sceneData.offsetBuffer;
	    sceneData.materialBuffer;
	    sceneData.areaLightBuffer;
	    sceneData.tlas;
	    sceneData.textures;
	    sceneData.envMap = envMap;
	    sceneData.envMapPdfBuffer = envMapPdfBuffer;

	    return USceneData(); // TODO
    }
    Buffer USceneBuilderBase::uploadInstanceData(CmdBuffer cmdBuf, IResourcePool *pPool)
    {
	    Buffer buf = createBuffer(pPool, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);
	    std::vector<VkAccelerationStructureInstanceKHR> instances = std::vector<VkAccelerationStructureInstanceKHR>(transformList.size());
	    for (uint32_t i = 0; i < instances.size(); i++)
        {
		    instances[i].transform = glmToVk(transformList[i]);
		    instances[i].instanceCustomIndex = i;
		    instances[i].mask = 0xFF;
		    instances[i].instanceShaderBindingTableRecordOffset = 0;
		    instances[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		    instances[i].accelerationStructureReference = modelList[i].blas->getDeviceAddress();
	    }
	    cmdWriteCopy(cmdBuf, buf, instances.data(), instances.size() * sizeof(VkAccelerationStructureInstanceKHR));
	    return buf;
    }
    void USceneBuilderBase::setTransform(std::string path, glm::mat4 transform)
    {
	    uint32_t index = indexMap.at(path);
	    transformList[index] = transform;
    }
    void USceneBuilderBase::reset()
    {
	    textureIndexMap.clear();
        modelList.clear();
        transformList.clear();
        indexMap.clear();
		envMap = nullptr;
    }
    void USceneBuilderBase::destroy()
    {
    }
    }        // namespace pbr
    }        // namespace vka
