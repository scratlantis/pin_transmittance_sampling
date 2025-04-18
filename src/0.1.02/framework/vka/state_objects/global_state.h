#pragma once
#include <vka/core/common.h>
#include "CmdAllocator.h"
#include "MemAllocator.h"
#include "Window.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vka/state_objects/IResourcePool.h>
#include <vka/state_objects/IResourceCache.h>


#define KEY_COUNT 1024

namespace vka
{

struct DeviceCI
{
	std::string               applicationName;
	uint32_t                  universalQueueCount;
	uint32_t                  computeQueueCount;
	std::vector<const char *> enabledInstanceExtensions;
	std::vector<const char *> enabledDeviceExtensions;
	StructureChain            enabledFeatures;
};

class Device
{
  public:
	VkDevice             logical;
	VkPhysicalDevice     physical;
	VkInstance           instance;
	std::vector<VkQueue> universalQueues;
	int                  universalQueueFamily;
	std::vector<VkQueue> computeQueues;
	int                  computeQueueFamily;

	Device();
	void configure(DeviceCI &deviceCI);
	void destroy();
	void createInstance();
	void selectPhysicalDevice();
	void createLogicalDevice();

	DELETE_COPY_CONSTRUCTORS(Device);

  private:
	DeviceCI deviceCI;
};

struct Mouse
{
	glm::vec2 change;
	glm::vec2 pos;
	bool      leftPressed;
	bool      leftEvent;
	bool      rightPressed;
	bool      rightEvent;
	// double    scrollOffset;
	double scrollChange;
	void   resetEvents()
	{
		leftEvent    = false;
		rightEvent   = false;
		change       = glm::vec2(0, 0);
		scrollChange = 0.0;
	}
};

struct IOControlerCI
{
	std::string                     windowTitel;
	bool                            resizable;
	VkExtent2D                      size;
	CursorMode                      cursorMode;
	std::vector<VkSurfaceFormatKHR> preferedFormats;
	std::vector<VkPresentModeKHR>   preferedPresentModes;
	uint32_t                        preferedSwapchainImageCount;

	WindowCI getWindowCI();
};

class SwapchainImage_I;
class IOController
{
  public:
	VkExtent2D         extent;
	VkSurfaceKHR       surface;
	VkFormat           format;
	VkSwapchainKHR     swapchain;
	VkPresentModeKHR   presentMode;
	uint32_t           imageCount;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkImageLayout> imageLayouts;
	VkImageUsageFlags  imageUsage;
	SwapchainImage_I *swapchainImage = nullptr;
	IResourcePool             *swapchainAttachmentPool;
	Mouse mouse;
	bool  keyPressed[KEY_COUNT];
	bool  keyEvent[KEY_COUNT];

	IOController();
	void    configure(IOControlerCI &controllerCI, Window *window);
	void    init();
	void    requestSwapchainRecreation();
	void    readInputs();
	void    destroy();
	void    terminateWindowManager();
	void    updateSwapchain();
	bool    swapchainRecreated();
	bool    shouldTerminate();
	Window *getWindow();

	DELETE_COPY_CONSTRUCTORS(IOController);

  private:
	bool               shouldRecreateSwapchain;
	bool               swapchainWasRecreated;
	Window            *window;
	IOControlerCI      controllerCI;
	SwapChainDetails   swapChainDetails;
	VkSurfaceFormatKHR surfaceFormat;
};

struct Frame
{
	VkSemaphore     renderFinishedSemaphore;
	VkSemaphore     imageAvailableSemaphore;
	VkFence         inFlightFence;
	uint32_t        frameIndex;
	IResourcePool*   stack;
	Frame          *next;
	Frame          *previous;
};

enum StateInitialisationBits
{
	STATE_INIT_DEVICE_INSTANCE_BIT = 0x1,
	STATE_INIT_DEVICE_PHYSICAL_BIT = 0x2,
	STATE_INIT_DEVICE_LOGICAL_BIT  = 0x4,
	STATE_INIT_DEVICE_BIT          = 0x8,
	STATE_INIT_IO_WINDOW_BIT       = 0x10,
	STATE_INIT_IO_SWAPCHAIN_BIT    = 0x20,
	STATE_INIT_IO_BIT              = 0x40,
	STATE_INIT_FRAME_SEMAPHORE_BIT = 0x80,
	STATE_INIT_FRAME_FENCE_BIT     = 0x100,
	STATE_INIT_FRAME_STACK_BIT     = 0x200,
	STATE_INIT_FRAME_BIT           = 0x400,
	STATE_INIT_HEAP_BIT            = 0x800,
	STATE_INIT_CACHE_BIT           = 0x1000,
	STATE_INIT_MEMALLOC_BIT        = 0x2000,
	STATE_INIT_DESCALLOC_BIT       = 0x4000,
	STATE_INIT_QUERYALLOC_BIT      = 0x8000,
	STATE_INIT_CMDALLOC_BIT        = 0x10000,
	STATE_INIT_ALL_BIT             = 0x20000
};

class AppState
{
  public:
	uint32_t       initBits;
	Device         device;
	IOController   io;
	IResourceCache *cache;
	Frame         *frame;
	MemAllocator    memAlloc;
	CmdAllocator   cmdAlloc;
	AppState();
	void init(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window *window);
	SubmitSynchronizationInfo acquireNextSwapchainImage();
	void                      presentFrame();
	void nextFrame();
	void destroy();

  private:
	std::vector<Frame> frames;
	void               initFrames();
	void               destroyFrames();

	DELETE_COPY_CONSTRUCTORS(AppState);
};

}        // namespace vka
extern vka::AppState            gState;
extern const std::string        gShaderOutputDir;
extern std::string				gShaderLog;
extern std::string              gVkaShaderPath;