#pragma once
#include "../resources/Resource.h"
namespace vka
{
class CmdBuffer;
void commitCmdBuffers(std::vector<CmdBuffer> cmdBufs, ResourceTracker *pTracker, VkQueue queue, const SubmitSynchronizationInfo syncInfo = {});

class CmdBuffer
{
  protected:
	NonUniqueResource *res;
  public:
	CmdBuffer();
	~CmdBuffer();


	void move(ResourceTracker *pNewTracker)
	{
		res->move(pNewTracker);
	}


	void end()
	{
		if (isRecording)
		{
			ASSERT_VULKAN(vkEndCommandBuffer(handle));
			isRecording = false;
		}
	}
	friend void commitCmdBuffers(std::vector<CmdBuffer> cmdBufs, ResourceTracker *pTracker, VkQueue queue, const SubmitSynchronizationInfo syncInfo);
  private:
	VkCommandBuffer handle;
	bool            isRecording;
};
}