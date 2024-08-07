#include "UScene.h"

namespace vka
{
namespace pbr
{
    USceneData USceneBuilderBase::create(CmdBuffer cmdBuf, IResourcePool *pPool)
    {
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
        textureNames.clear();
        modelList.clear();
        transformList.clear();
        indexMap.clear();
    }
    }        // namespace pbr
    }        // namespace vka
