#pragma once
#include <vka/core/core_common.h>

namespace vka
{
class TimeQueryManager
{
  public:
	TimeQueryManager(IResourcePool *pPool, uint32_t queryCount);
	TimeQueryManager() = default;
	~TimeQueryManager();

	void cmdResetQueryPool(CmdBuffer cmdBuffer);
	void startTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage);

	void endTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage);

	bool updateTimings();
	void garbageCollect();

	std::vector<float> timings;

  private:
	Resource *queryPoolRes = nullptr;
	VkQueryPool           queryPool;
	std::vector<uint64_t> queryResults;
	bool firstUse = true;
};
}        // namespace vka