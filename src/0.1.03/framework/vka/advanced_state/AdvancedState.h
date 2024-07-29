#pragma once
#include <vka/core/core_state/CoreState.h>
#include "caches/FramebufferCache.h"
#include "caches/ModelCache.h"
#include "caches/TextureCache.h"

namespace vka
{
struct AdvancedStateConfig
{
	std::string        modelPath;
	std::string        texturePath;
	VkBufferUsageFlags modelUsage;
};

class AdvancedState : public CoreState
{

  public:
	FramebufferCache *framebufferCache;
	ModelCache       *modelCache;
	TextureCache     *textureCache;
	IResourcePool    *heap;
	IResourcePool    *swapchainAttachmentPool;
	SwapchainImage_R *swapchainImage;


	AdvancedState(){};
	~AdvancedState(){};
	void init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AdvancedStateConfig &config);
	void destroy() override;
	virtual void nextFrame() override;
};
}        // namespace vka
