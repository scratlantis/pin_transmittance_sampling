#pragma once
#include <vka/vka.h>
const uint32_t numTimings = 2;
enum TimeQueryIds
{
	TQ_PATH_TRACER_A,
	TQ_PATH_TRACER_B
};

class TimeQueryManager
{
  public:
	TimeQueryManager(std::vector<TimeQueryIds> timeQueryIds);
	TimeQueryManager() = default;
	void cmdResetQueryPool(CmdBuffer cmdBuffer);
	~TimeQueryManager();

	void startTiming(CmdBuffer cmdBuffer, TimeQueryIds queryID, VkPipelineStageFlagBits stage);

	void endTiming(CmdBuffer cmdBuffer, TimeQueryIds queryID, VkPipelineStageFlagBits stage);

	bool updateTimings();


	void destroy();

	std::map<TimeQueryIds, float> timings;

  private:
	uint32_t                         numTimings;
	VkQueryPool                      queryPool;
	std::vector<uint64_t>            queryResults;
	std::map<TimeQueryIds, uint32_t> timingIdx;
};
