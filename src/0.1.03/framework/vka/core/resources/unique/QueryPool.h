#pragma once
#include "../Resource.h"
namespace vka
{
	/*class QueryPoolVk_R : public Resource_T<VkQueryPool>
	{
		void free() override
		{
			vkDestroyQueryPool(gState.device.logical, this->handle, nullptr);
		}

	  public:
		QueryPoolVk_R(VkQueryPool handle) :
		    Resource_T<VkQueryPool>(handle)
		{}
	};

	class QueryPool_R : public Resource_T<VkQueryPool>
    {
	  protected:
	    QueryPoolVk_R *res = nullptr;
		std::vector<uint64_t> results;
	  public:

	  QueryPool_R(IResourcePool *pPool, uint32_t size, VkQueryType type) :
	        Resource_T<VkQueryPool>(VK_NULL_HANDLE)
	  {
		  VkQueryPoolCreateInfo ci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
		  ci.queryType  = type;
		  ci.queryCount = size;
		  VK_CHECK(vkCreateQueryPool(gState.device.logical, &ci, nullptr, &handle));
		  results.resize(size * 2);
		  res = new QueryPoolVk_R(handle);
		  res->track(pPool);
	  }
	  void free() override {}



	  private:
    };*/

    }