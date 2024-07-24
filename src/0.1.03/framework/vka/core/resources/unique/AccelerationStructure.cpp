#include "AccelerationStructure.h"
#include <vka/globals.h>
#include <vka/core/core_state/device_properties.h>


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
}
void BottomLevelAS_R::configureScratchBuffer(Buffer_R *scratchBuffer) const
{
	VkPhysicalDeviceAccelerationStructurePropertiesKHR asProp = getAccelerationStructureProperties();
	scratchBuffer->changeSize(std::min(scratchBuffer->getSize(), getBuildSize() + asProp.minAccelerationStructureScratchOffsetAlignment));
	scratchBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	scratchBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
}

//VkDeviceSize BottomLevelAS_R::getBuildScratchSize() const
//{
//	return buildScratchSize;
//};
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

VkAccelerationStructureBuildGeometryInfoKHR BottomLevelAS_R::getBuildInfo() const
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
	buildInfo.flags                    = buildFlags;
	buildInfo.geometryCount            = geometry.size();
	buildInfo.pGeometries              = geometry.data();
	buildInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	return buildInfo;
}

VkAccelerationStructureBuildGeometryInfoKHR BottomLevelAS_R::getBuildInfo(VkAccelerationStructureKHR src, Buffer_R *scratchBuffer) const
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfo();
	buildInfo.srcAccelerationStructure                    = src;
	buildInfo.mode                                        = (src == VK_NULL_HANDLE) ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	buildInfo.dstAccelerationStructure                    = handle;
	buildInfo.scratchData.deviceAddress                   = scratchBuffer->getDeviceAddress();
	return VkAccelerationStructureBuildGeometryInfoKHR();
}


VkDeviceSize BottomLevelAS_R::getBuildSize() const
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfo();
	std::vector<uint32_t> maxPrimCount(buildRange.size());
	for (uint32_t i = 0; i < buildRange.size(); i++)
	{
		maxPrimCount[i] = buildRange[i].primitiveCount;
	}
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
	vkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, maxPrimCount.data(), &sizeInfo);
}

void BottomLevelAS_R::createHandles()
{
    VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	createInfo.size = getBuildSize();
	bufRes->changeSize(createInfo.size);
	bufRes->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	bufRes->recreate();
	createInfo.buffer = bufRes->getHandle();
    VK_CHECK(vkCreateAccelerationStructureKHR(gState.device.logical, &createInfo, nullptr, &handle));
	asRes = new AccelerationStructure_R(handle);
	asRes->track(pPool);
}


void TopLevelAs_R::configureScratchBuffer(Buffer_R *scratchBuffer) const
{
	VkPhysicalDeviceAccelerationStructurePropertiesKHR asProp = getAccelerationStructureProperties();
	scratchBuffer->changeSize(std::min(scratchBuffer->getSize(), getBuildSize() + asProp.minAccelerationStructureScratchOffsetAlignment));
	scratchBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	scratchBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
}

bool TopLevelAs_R::isBuilt() const
{
	return built;
}

void TopLevelAs_R::setBuilt(bool built)
{
	this->built = built;
}

void TopLevelAs_R::setBuildFlags(VkBuildAccelerationStructureFlagsKHR flags)
{
	buildFlags = flags;
}

void TopLevelAs_R::setInstanceCount(uint32_t count)
{
	instanceCount = count;
}

void TopLevelAs_R::track(IResourcePool *pPool)
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
hash_t TopLevelAs_R::hash() const
{
	return asRes->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}
void TopLevelAs_R::detachChildResources()
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
void TopLevelAs_R::recreate()
{
	detachChildResources();
	createHandles();
	built = false;
}
const TopLevelAs_R TopLevelAs_R::getShallowCopy() const
{
	return *this;
}
VkAccelerationStructureBuildGeometryInfoKHR TopLevelAs_R::getBuildInfo() const
{
	VkAccelerationStructureGeometryInstancesDataKHR instancesVk{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
	VkAccelerationStructureGeometryKHR              topASGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
	topASGeometry.geometryType       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	topASGeometry.geometry.instances = instancesVk;
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
	buildInfo.flags         = buildFlags;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries   = &topASGeometry;
	buildInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	return buildInfo;
}

VkDeviceSize TopLevelAs_R::getBuildSize() const
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfo();
	VkAccelerationStructureBuildSizesInfoKHR    sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
	vkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &instanceCount, &sizeInfo);
	return sizeInfo.accelerationStructureSize;
}

void TopLevelAs_R::createHandles()
{
	VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	createInfo.size = getBuildSize();
	bufRes->changeSize(createInfo.size);
	bufRes->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	bufRes->recreate();
	createInfo.buffer = bufRes->getHandle();
	VK_CHECK(vkCreateAccelerationStructureKHR(gState.device.logical, &createInfo, nullptr, &handle));
	asRes = new AccelerationStructure_R(handle);
	asRes->track(pPool);
}
}        // namespace vka