#include "TimeQueryManager.h"

TimeQueryManager::TimeQueryManager(std::vector<TimeQueryIds> timeQueryIds)
{
	numTimings = timeQueryIds.size();
	for (size_t i = 0; i < timeQueryIds.size(); i++)
	{
		timingIdx[timeQueryIds[i]] = i;
		timings[timeQueryIds[i]]   = 0.0f;
	}
	queryResults.resize(numTimings * 2);
	VkQueryPoolCreateInfo queryPoolCI{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
	queryPoolCI.queryType  = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolCI.queryCount = queryResults.size();
	queryPoolCI.pipelineStatistics;

	vkCreateQueryPool(gState.device.logical, &queryPoolCI, nullptr, &queryPool);
}

void TimeQueryManager::cmdResetQueryPool(CmdBuffer cmdBuffer)
{
	vkCmdResetQueryPool(cmdBuffer->getHandle(), queryPool, 0, queryResults.size());
}

TimeQueryManager::~TimeQueryManager()
{
}

void TimeQueryManager::startTiming(CmdBuffer cmdBuffer, TimeQueryIds queryID, VkPipelineStageFlagBits stage)
{
	vkCmdWriteTimestamp(cmdBuffer->getHandle(), stage, queryPool, timingIdx[queryID] * 2);
}

void TimeQueryManager::endTiming(CmdBuffer cmdBuffer, TimeQueryIds queryID, VkPipelineStageFlagBits stage)
{
	vkCmdWriteTimestamp(cmdBuffer->getHandle(), stage, queryPool, timingIdx[queryID] * 2 + 1);
}

bool TimeQueryManager::updateTimings()
{
	VkResult result = vkGetQueryPoolResults(
	    gState.device.logical,
	    queryPool,
	    0,
	    queryResults.size(),
	    queryResults.size() * sizeof(uint64_t),
	    queryResults.data(),
	    static_cast<VkDeviceSize>(sizeof(uint64_t)),
	    VK_QUERY_RESULT_64_BIT);
	VkPhysicalDeviceProperties physicalDeviceProps{};
	vkGetPhysicalDeviceProperties(gState.device.physical, &physicalDeviceProps);
	if (result != VK_NOT_READY)
	{
		std::map<TimeQueryIds, uint32_t>::iterator it;
		for (it = timingIdx.begin(); it != timingIdx.end(); it++)
		{
			uint32_t idx       = it->second;
			uint64_t t_ns      = (queryResults[2 * idx + 1] - queryResults[2 * idx]) * physicalDeviceProps.limits.timestampPeriod;
			float    t_float   = static_cast<float>(t_ns) / 1000000.f;
			timings[it->first] = t_float;
		}
	}
	return result != VK_NOT_READY;
}

void TimeQueryManager::destroy()
{
	vkDestroyQueryPool(gState.device.logical, queryPool, nullptr);
}
