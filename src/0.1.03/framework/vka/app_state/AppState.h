#pragma once
#include <vka/core/core_state/CoreState.h>
#include "caches/FramebufferCache.h"
#include "caches/ModelCache.h"
#include "caches/TextureCache.h"

namespace vka
{
struct AppStateConfig
{
	std::string        modelPath;
	std::string        texturePath;
	VkBufferUsageFlags modelUsage;
};

class AppState : public CoreState
{

  public:
	FramebufferCache *framebufferCache;
	ModelCache       *modelCache;
	TextureCache     *textureCache;
	IResourcePool    *heap;
	IResourcePool    *swapchainAttachmentPool;
	SwapchainImage_R *swapchainImage;


	AppState(){};
	~AppState(){};
	void init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AppStateConfig &config);
	void destroy() override;
	virtual void nextFrame() override;
};
}        // namespace vka
