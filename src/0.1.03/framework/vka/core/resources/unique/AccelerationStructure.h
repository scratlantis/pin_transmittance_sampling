#pragma once
#include "../Resource.h"
#include "Buffer.h"

namespace vka
{

class AccelerationStructure_R : public Resource_T<VkAccelerationStructureKHR>
{
  public:
	AccelerationStructure_R(VkAccelerationStructureKHR handle) :
	    Resource_T<VkAccelerationStructureKHR>(handle){};

  protected:
	void free();

  private:
	VkAccelerationStructureKHR handle;
};

class BottomLevelAS_R : public Resource_T<VkAccelerationStructureKHR>
{
  protected:
	Resource_T<VkAccelerationStructureKHR> *asRes  = nullptr;
	Buffer_R                               *bufRes = nullptr;

	VkDeviceSize                                          buildScratchSize = 0;
	bool                                                  built            = false;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
	std::vector<VkAccelerationStructureGeometryKHR>       geometry;
	VkBuildAccelerationStructureFlagsKHR                  buildFlags;

  private:
	BottomLevelAS_R &operator=(const BottomLevelAS_R &rhs)
	{
		pPool = nullptr;

		asRes  = rhs.asRes;
		bufRes = rhs.bufRes;

		buildScratchSize = rhs.buildScratchSize;
		built            = rhs.built;
		buildRange       = rhs.buildRange;
		geometry         = rhs.geometry;
		buildFlags       = rhs.buildFlags;
	};

  public:
	BottomLevelAS_R() = default;
	BottomLevelAS_R(IResourcePool *pPool) :
	    Resource_T<VkAccelerationStructureKHR>(VK_NULL_HANDLE)
	{
		bufRes = new Buffer_R(pPool, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		Resource::track(pPool);
	};


	const VkAccelerationStructureGeometryKHR                     *getGeometryPtr() const;
	uint32_t                                                      getGeometryCount() const;
	std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> getBuildRangePtrs() const;
	VkDeviceSize                                                  getBuildScratchSize() const;
	bool                                                          isBuilt() const;

	void setGeometry(std::vector<VkAccelerationStructureGeometryKHR> &geom);
	void setBuildRange(std::vector<VkAccelerationStructureBuildRangeInfoKHR> &range);
	void setBuilt(bool built);
	void setBuildFlags(VkBuildAccelerationStructureFlagsKHR flags);

	void   free(){};
	void   track(IResourcePool *pPool) override;
	hash_t hash() const override;

	void                  createHandles();
	void                  detachChildResources();
	void                  recreate();
	const BottomLevelAS_R getShallowCopy() const;

  
};

}