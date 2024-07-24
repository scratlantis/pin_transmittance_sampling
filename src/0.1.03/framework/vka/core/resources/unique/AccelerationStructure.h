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

	bool                                                  built            = false;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
	std::vector<VkAccelerationStructureGeometryKHR>       geometry;
	VkBuildAccelerationStructureFlagsKHR                  buildFlags = 0;

	BottomLevelAS_R &operator=(const BottomLevelAS_R &rhs)
	{
		pPool = nullptr;

		asRes  = rhs.asRes;
		bufRes = rhs.bufRes;

		built            = rhs.built;
		buildRange       = rhs.buildRange;
		geometry         = rhs.geometry;
		buildFlags       = rhs.buildFlags;
	};

	VkAccelerationStructureBuildGeometryInfoKHR getBuildInfo() const;
	VkDeviceSize                                getBuildSize() const;

  public:
	BottomLevelAS_R() = default;
	BottomLevelAS_R(IResourcePool *pPool) :
	    Resource_T<VkAccelerationStructureKHR>(VK_NULL_HANDLE)
	{
		bufRes = new Buffer_R(pPool, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		Resource::track(pPool);
	};

	VkAccelerationStructureBuildGeometryInfoKHR                   getBuildInfo(VkAccelerationStructureKHR src, Buffer_R *scratchBuffer) const;
	const VkAccelerationStructureGeometryKHR                     *getGeometryPtr() const;
	uint32_t                                                      getGeometryCount() const;
	std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> getBuildRangePtrs() const;
	void                                                          configureScratchBuffer(Buffer_R *scratchBuffer) const;
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

class TopLevelAs_R : public Resource_T<VkAccelerationStructureKHR>
{
  protected:
	Resource_T<VkAccelerationStructureKHR> *asRes  = nullptr;
	Buffer_R                               *bufRes = nullptr;

	bool                                                  built            = false;
	uint32_t                                              instanceCount    = 0;
	VkBuildAccelerationStructureFlagsKHR                  buildFlags;

  private:
	TopLevelAs_R &operator=(const TopLevelAs_R &rhs)
	{
		pPool = nullptr;

		asRes  = rhs.asRes;
		bufRes = rhs.bufRes;

		built            = rhs.built;
		buildFlags       = rhs.buildFlags;
	};

	VkAccelerationStructureBuildGeometryInfoKHR getBuildInfo() const;
	VkDeviceSize                                getBuildSize() const;
  public:
	TopLevelAs_R() = default;
	TopLevelAs_R(IResourcePool *pPool) :
	    Resource_T<VkAccelerationStructureKHR>(VK_NULL_HANDLE)
	{
		bufRes = new Buffer_R(pPool, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		Resource::track(pPool);
	};

	VkAccelerationStructureBuildGeometryInfoKHR getBuildInfo(VkAccelerationStructureKHR src, Buffer_R *scratchBuffer) const;
	void                                configureScratchBuffer(Buffer_R* scratchBuffer) const;
	bool                                        isBuilt() const;

	void setBuilt(bool built);
	void setBuildFlags(VkBuildAccelerationStructureFlagsKHR flags);
	void setInstanceCount(uint32_t count);


	void   free(){};
	void   track(IResourcePool *pPool) override;
	hash_t hash() const override;

	void               createHandles();
	void               detachChildResources();
	void               recreate();
	const TopLevelAs_R getShallowCopy() const;
};

}