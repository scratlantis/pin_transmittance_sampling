#include "AppState.h"
#include <vka/core/core_state/ResourcePool.h>
#include <vka/core_interface/cmd_buffer_utility.h>
#include <vka/core_interface/general_commands.h>
namespace vka
{
void AppState::init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AppStateConfig &config)
{
	CoreState::init(deviceCI, ioControllerCI, window);
	swapchainAttachmentPool = new ResourcePool();
	heap                    = new ResourcePool();
	framebufferCache        = new FramebufferCache();
	modelCache              = new ModelCache(heap, config.modelPath, config.modelUsage);
	textureCache            = new TextureCache(heap, config.texturePath);
	swapchainImage          = nullptr;
}

void AppState::destroy()
{
	heap->clear();
	swapchainAttachmentPool->clear();
	delete framebufferCache;
	delete modelCache;
	delete textureCache;
	delete heap;
	delete swapchainAttachmentPool;
	if (swapchainImage != nullptr)
	{
		delete swapchainImage;
	}
	CoreState::destroy();
}

void AppState::nextFrame()
{
	CoreState::nextFrame();
	if (io.swapchainRecreated())
	{
		if (initBits & STATE_INIT_ALL_BIT)
		{
			// Resize swapchain attachments
			CmdBuffer cmdBuf = createCmdBuffer(frame->stack);
			for (auto it = swapchainAttachmentPool->getImagesBegin(); it != swapchainAttachmentPool->getImagesEnd(); ++it)
			{
				(*it)->changeExtent({io.extent.width, io.extent.height, 1});
				(*it)->recreate();
				cmdTransitionLayout(cmdBuf, (*it), (*it)->getInitialLayout());
			}
			executeImmediat(cmdBuf);
		}
	}
}
}        // namespace vka