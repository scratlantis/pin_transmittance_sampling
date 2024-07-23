#include "AccelerationStructure.h"
#include <vka/globals.h>


namespace vka
{
void AccelerationStructure_R::free()
{
	vkDestroyAccelerationStructureKHR(gState.device.logical, handle, nullptr);
}

const VkAccelerationStructureGeometryKHR *BottomLevelAS_R::getGeometryPtr() const
{
	return geometry.data();
};

uint32_t BottomLevelAS_R::getGeometryCount() const
{
	return geometry.size();
};

std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> BottomLevelAS_R::getBuildRangePtrs() const
{
	std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> ptrs;
	for (auto &range : buildRange)
	{
		ptrs.push_back(&range);
	}
	return ptrs;
};

VkDeviceSize BottomLevelAS_R::getBuildScratchSize() const
{
	return buildScratchSize;
};
bool BottomLevelAS_R::isBuilt() const
{
	return isBuilt;
};

void BottomLevelAS_R::setGeometry(std::vector<VkAccelerationStructureGeometryKHR> &geom)
{
	geometry = geom;
};

void BottomLevelAS_R::setBuildRange(std::vector<VkAccelerationStructureBuildRangeInfoKHR> &range)
{
	buildRange = range;
};

void BottomLevelAS_R::setBuilt(bool built)
{
	this->built = built;
};

void BottomLevelAS_R::setBuildFlags(VkBuildAccelerationStructureFlagsKHR flags)
{
	buildFlags = flags;
};



void BottomLevelAS_R::track(IResourcePool *pPool)
{
	if (!pPool)
	{
		printVka("Null resource pool\n");
		DEBUG_BREAK;
		return;
	}
	if (asRes)
	{
		asRes->track(pPool);
	}
	if (bufRes)
	{
		bufRes->track(pPool);
	}
	Resource::track(pPool);
}
hash_t BottomLevelAS_R::hash() const
{
	return asRes->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}
void BottomLevelAS_R::detachChildResources()
{
	if (asRes)
	{
		asRes->track(gState.frame->stack);
		asRes = nullptr;
	}
	if (bufRes)
	{
		bufRes->track(gState.frame->stack);
		bufRes = nullptr;
	}
}
void BottomLevelAS_R::recreate()
{
	detachChildResources();
	createHandles();
	built = false;
}
const BottomLevelAS_R BottomLevelAS_R::getShallowCopy() const
{
	return *this;
}

void BottomLevelAS_R::createHandles()
{
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
    buildInfo.flags                    = buildFlags;
    buildInfo.geometryCount            = geometry.size();
	buildInfo.pGeometries              = geometry.data();
    buildInfo.mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

    // Get sizes
    std::vector<uint32_t> maxPrimCount(buildRange.size());
    for (uint32_t i = 0; i < buildRange.size(); i++)
    {
		maxPrimCount[i] = buildRange[i].primitiveCount;
    }
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    vkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, maxPrimCount.data(), &sizeInfo);

	// Create acceleration structure
    VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    createInfo.size = sizeInfo.accelerationStructureSize;
	bufRes->changeSize(createInfo.size);
	bufRes->changeMemoryType(VMA_MEMORY_USAGE_CPU_ONLY);
	bufRes->recreate();
	createInfo.buffer = bufRes->getHandle();
    VK_CHECK(vkCreateAccelerationStructureKHR(gState.device.logical, &createInfo, nullptr, &handle));
	asRes = new AccelerationStructure_R(handle);
	asRes->track(pPool);
}
}        // namespace vka