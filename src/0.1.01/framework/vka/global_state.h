#pragma once
#include "common.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include "core/container/StructureChain.h"
#include "allocators/CmdAllocator.h"
#include "allocators/MemAllocator.h"
#include "allocators/QueryAllocator.h"
#include "allocators/DescriptorAllocator.h"
#include "global_var.h"

#define KEY_COUNT 1024




namespace vka
{

template <class T>
struct PointerObjHash
{
	bool operator()(T const *a) const
	{
		std::hash<T> h;
		return h(*a);
	}
};
template <class T>
struct PointerObjEq
{
	bool operator()(T const *a, T const *b) const
	{
		return *a == *b;
	}
};

class ResourceTracker
{
  public:
	ResourceTracker(){};
	~ResourceTracker(){};

	Resource *find(Resource *resource) const;
	void      clear();
	bool	  move(Resource *resource, ResourceTracker *next);
	bool      add(Resource *resource);
	bool	  remove(Resource *resource);
  private:
	std::unordered_set<Resource *, PointerObjHash<Resource>, PointerObjEq<Resource>> resources;
};

class Resource
{
  protected:
	ResourceTracker* pTracker;
	virtual void free() = 0;
	virtual bool _equals(Resource const &other) const = 0;
  public:
	//{
	//	std::string a = typeid(*this).name();
	//	std::string b = typeid(other).name();
	//	if (typeid(*this) != typeid(other))
	//	{
	//		return false;
	//	}
	//	else
	//	{
	//		DEBUG_BREAK
	//		return true;
	//	}
	//}
	virtual hash_t _hash() const = 0;
	friend class ResourceTracker;

	bool operator==(Resource const &other) const
	{
		std::string a = typeid(*this).name();
		std::string b = typeid(other).name();
		if (typeid(*this) != typeid(other))
		{
			return false;
		}
		else
		{
			return this->_equals(other);
		}
	}

	void move(ResourceTracker *pNewTracker)
	{
		Resource *result = pTracker->find(this);
		if (result)
		{
			pTracker->move(result, pNewTracker);
		}
	}
};

}		// namespace vka
namespace std
{
template <>
struct hash<vka::Resource>
{
	size_t operator()(vka::Resource const &r) const
	{
		return static_cast<size_t>(r._hash());
	}
};
}        // namespace std

namespace vka
{
struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR        surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentationMode;
};

enum CursorMode
{
	VISIBLE,
	HIDDEN,
	DISABLED
};

struct WindowCI
{
	uint32_t    width;
	uint32_t    height;
	std::string title;
	bool        resizable;
	CursorMode  cursorMode;
};

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
	VkDevice         logical;
	VkPhysicalDevice physical;
	VkInstance       instance;
	std::vector<VkQueue> universalQueues;
	int universalQueueFamily;
	std::vector<VkQueue> computeQueues;
	int computeQueueFamily;

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
	//double    scrollOffset;
	double    scrollChange;
	void resetEvents()
	{
		leftEvent = false;
		rightEvent = false;
		change     = glm::vec2(0, 0);
		scrollChange = 0.0;
	}
};



class Window
{
  public:
	virtual ~Window(){};
	virtual void                      initWindowManager() = 0;
	virtual void                      terminateWindowManager() = 0;
	virtual void                      init(const WindowCI &windowCI, VkInstance &instance) = 0;
	virtual void                      pollEvents()                                  = 0;
	virtual void                      waitEvents()                                  = 0;
	virtual bool                      shouldClose()                                 = 0;
	virtual void                      requestClose()                                = 0;
	virtual VkExtent2D                size() const                                  = 0;
	virtual void                      changeSize(VkExtent2D newSize)                = 0;
	virtual VkSurfaceKHR              getSurface() const                            = 0;
	virtual void                      destroy()                                     = 0;
	virtual void                      initGui()                                     = 0;
	virtual void                      addInstanceExtensions(std::vector<const char *> &extensions) = 0;
  private:
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


class IOController
{
  public:
	VkExtent2D               extent;
	VkSurfaceKHR             surface;
	VkFormat                 format;
	VkSwapchainKHR           swapchain;
	VkPresentModeKHR         presentMode;
	uint32_t                 imageCount;
	std::vector<Image>	     images;
	//std::vector<VkImageView> imageViews;
	Mouse                    mouse;
	bool                     keyPressed[KEY_COUNT];
	bool                     keyEvent[KEY_COUNT];

	IOController();
	void configure(IOControlerCI &controllerCI, Window *window);
	void init();
	void requestSwapchainRecreation();
	void readInputs();
	void destroy();
	void terminateWindowManager();
	bool shouldTerminate();
	bool updateSwapchain();
	Window *getWindow();

	DELETE_COPY_CONSTRUCTORS(IOController);

  private:
	bool               shouldRecreateSwapchain;
	Window            *window;
	IOControlerCI      controllerCI;
	SwapChainDetails   swapChainDetails;
	VkSurfaceFormatKHR surfaceFormat;
};

struct Frame
{
	VkSemaphore renderFinishedSemaphore;
	VkSemaphore imageAvailableSemaphore;
	VkFence     inFlightFence;
	uint32_t    frameIndex;
	ResourceTracker stack;
	Frame *next;
	Frame *previous;
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
	uint32_t            initBits;
	Device              device;
	IOController        io;
	ResourceTracker     heap;
	ResourceTracker     cache;
	Frame              *frame;
	MemAllocator        memAlloc;
	DescriptorAllocator descAlloc;
	QueryAllocator      queryAlloc;
	CmdAllocator        cmdAlloc;

	AppState();
	void init(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window* window);
	void swapBuffers(CmdBuffer *cmdBufs, uint32_t cmdBufCount = 1);
	void nextFrame();
	void destroy();
	bool swapchainRecreated()
	{
		return swapchainWasRecreated;
	}
	private:
	void endFrame(CmdBuffer *cmdBufs, uint32_t cmdBufCount = 1);
	std::vector<Frame> frames;
	bool               swapchainWasRecreated;
	void initFrames();
	void destroyFrames();

	DELETE_COPY_CONSTRUCTORS(AppState);
};


}        // namespace vka
extern vka::AppState gState;
extern const std::string gShaderOutputDir;
extern const std::string gResourceBaseDir;
extern std::vector<vka::GVar *> gVars;