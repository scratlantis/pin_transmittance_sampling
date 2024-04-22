#pragma once
#include "../resources/Resource.h"
#include "Buffer.h"
namespace vka
{
// Can be updated (rebuild)
class BLAS
{
  protected:
	NonUniqueResource *asRes;

  public:
	VkAccelerationStructureKHR                            as;
	Buffer                                                buffer;
	VkDeviceSize                                          buildScratchSize;
	bool                                                  isBuilt;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
	std::vector<VkAccelerationStructureGeometryKHR>       geometry;

	BLAS(
	    ResourceTracker                                       *pTracker,
	    std::vector<VkAccelerationStructureGeometryKHR>       &geomKHR,
	    std::vector<VkAccelerationStructureBuildRangeInfoKHR> &buildRangeKHR,
	    VkBuildAccelerationStructureFlagsKHR                   flags)
	{
		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
		buildInfo.flags                    = flags;
		buildInfo.geometryCount            = geomKHR.size();
		buildInfo.pGeometries              = geomKHR.data();
		buildInfo.mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

		std::vector<uint32_t> maxPrimCount(buildRangeKHR.size());

		for (uint32_t i = 0; i < buildRangeKHR.size(); i++)
		{
			maxPrimCount[i] = buildRangeKHR[i].primitiveCount;
		}

		// Get sizes
		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
		vkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		                                        &buildInfo, maxPrimCount.data(), &sizeInfo);

		// Create acceleration structure object. Not yet bound to memory.
		VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
		createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		createInfo.size = sizeInfo.accelerationStructureSize;
		// As buffer
		buffer            = BufferDedicated(pTracker,
		                                    createInfo.size,
		                                    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		createInfo.buffer = buffer.buf;
		// Create the acceleration structure
		ASSERT_VULKAN(vkCreateAccelerationStructureKHR(gState.device.logical, &createInfo, nullptr, &as));
		asRes = new AccelerationStructure_R(as);
		pTracker->add(asRes);

		buildScratchSize = sizeInfo.buildScratchSize;
		isBuilt          = false;

		geometry   = geomKHR;
		buildRange = buildRangeKHR;
	}

  private:
};

// Does not support updating. Must be recreated.
struct TLAS
{
	NonUniqueResource                          *asRes;
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo;
	VkDeviceSize                                buildSize;

  public:
	VkAccelerationStructureKHR as;
	Buffer                     asBuffer;
	Buffer                     instanceBuffer;
	Buffer                     descriptorBuffer;
	bool                       isBuilt;

	TLAS(
	    ResourceTracker                                       *pTracker,
	    const std::vector<VkAccelerationStructureInstanceKHR> &instancesKHR,
	    VkBuildAccelerationStructureFlagsKHR                   flags)
	{
		// Create instance buffer
		instanceBuffer = BufferVma(pTracker, dataSize(instancesKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);

		// Get device address
		VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
		bufferInfo.buffer               = instanceBuffer.buf;
		VkDeviceAddress instanceAddress = vkGetBufferDeviceAddress(gState.device.logical, &bufferInfo);

		// Create tlas
		VkAccelerationStructureGeometryInstancesDataKHR instancesVk{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
		instancesVk.arrayOfPointers    = VK_FALSE;
		instancesVk.data.deviceAddress = instanceAddress;

		VkAccelerationStructureGeometryKHR topASGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
		topASGeometry.geometryType       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		topASGeometry.geometry.instances = instancesVk;

		// Find sizes
		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
		buildInfo.flags         = flags;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries   = &topASGeometry;
		buildInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

		uint32_t                                 count = (uint32_t) instancesKHR.size();
		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};

		/* The srcAccelerationStructure, dstAccelerationStructure, and mode members of pBuildInfo are ignored.Any VkDeviceOrHostAddressKHR
		        or VkDeviceOrHostAddressConstKHR members of pBuildInfo are ignored by this command, except that the hostAddress member of
		        VkAccelerationStructureGeometryTrianglesDataKHR::transformData will be examined to check if it is NULL.*/
		vkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &count, &sizeInfo);

		buildSize = sizeInfo.accelerationStructureSize;

		VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
		createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		createInfo.size = buildSize;

		// Create as buffer
		asBuffer          = BufferVma(pTracker, createInfo.size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		createInfo.buffer = asBuffer.buf;

		// Create tlas
		vkCreateAccelerationStructureKHR(gState.device.logical, &createInfo, nullptr, &as);

		//// Query min scratch alignment
		// VkPhysicalDeviceAccelerationStructurePropertiesKHR pAsProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
		// VkPhysicalDeviceProperties2                        pProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
		// pProperties.pNext = &pAsProperties;
		// vkGetPhysicalDeviceProperties2(physDevice, &pProperties);
		// uint32_t minScratchOffsetAlign = pAsProperties.minAccelerationStructureScratchOffsetAlignment;

		//// Create scratch buffer for build
		// scratchBuffer = createBufferVma(
		//     allocator,
		//     createInfo.size + minScratchOffsetAlign,
		//     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		//     VMA_MEMORY_USAGE_GPU_ONLY);

		//// Get scratch buffer address
		// bufferInfo.buffer              = scratchBuffer.buf;
		// VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(device, &bufferInfo);
		// scratchAddress                 = alignUp(scratchAddress, minScratchOffsetAlign);
		//// Update build information
		// buildInfo.srcAccelerationStructure  = VK_NULL_HANDLE;
		// buildInfo.dstAccelerationStructure  = tlas.as;
		// buildInfo.scratchData.deviceAddress = scratchAddress;

		//// Build Offsets info
		// VkAccelerationStructureBuildRangeInfoKHR        buildOffsetInfo{static_cast<uint32_t>(instancesKHR.size()), 0, 0, 0};
		// const VkAccelerationStructureBuildRangeInfoKHR *pBuildOffsetInfo = &buildOffsetInfo;

		//// Build the TLAS
		// pvkCmdBuildAccelerationStructuresKHR(cmdBuf, 1, &buildInfo, &pBuildOffsetInfo);

		// barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		// barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		// vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		//                      VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		// return tlas;
	};
};
}        // namespace vka