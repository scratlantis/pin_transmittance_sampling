#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
namespace vka
{

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

class Window
{
  public:
	virtual ~Window(){};
	virtual void         initWindowManager()                                          = 0;
	virtual void         terminateWindowManager()                                     = 0;
	virtual void         init(const WindowCI &windowCI, VkInstance &instance)         = 0;
	virtual void         pollEvents()                                                 = 0;
	virtual void         waitEvents()                                                 = 0;
	virtual bool         shouldClose()                                                = 0;
	virtual void         requestClose()                                               = 0;
	virtual VkExtent2D   size() const                                                 = 0;
	virtual void         changeSize(VkExtent2D newSize)                               = 0;
	virtual VkSurfaceKHR getSurface() const                                           = 0;
	virtual void         destroy()                                                    = 0;
	virtual void         initGui()                                                    = 0;
	virtual void         addInstanceExtensions(std::vector<const char *> &extensions) = 0;

  private:
};

}