#pragma once
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "../global_state.h"
#include "../core/macros/macros.h"
namespace vka
{


class GlfwWindow : public Window
{
  public:

	GlfwWindow();
	void initWindowManager();
	void terminateWindowManager();

	void init(const WindowCI &windowCI, VkInstance &instance);
	VkExtent2D   size() const;
	VkSurfaceKHR getSurface() const;
	void         pollEvents();
	void         waitEvents();
	bool         shouldClose();
	void         requestClose();
	void         changeSize(VkExtent2D newSize);
	void		 addInstanceExtensions(std::vector<const char *> &extensions); 
	void         destroy();

	~GlfwWindow();

  private:
	GLFWwindow  *window;
	GLint        width, height;
	bool         keyPressedLastFrame[KEY_COUNT];
	bool         keyPressed[KEY_COUNT];
	glm::vec2    mousePosLastFrame;
	glm::vec2    mousePos;
	bool         mouseLeftPressedLastFrame;
	bool         mouseLeftPressed;
	bool         mouseRightPressedLastFrame;
	bool         mouseRightPressed;
	double       scrollOffsetLastFrame;
	double       scrollOffset;
	VkSurfaceKHR surface;

	static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
	static void key_callback(GLFWwindow *window, int key, int code, int action, int mode);
	static void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
	static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
	static void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
};



void GlfwWindow::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	GlfwWindow *thisWindow = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
	thisWindow->width             = width;
	thisWindow->height            = height;
	state.io.requestSwapchainRecreation();
}

void GlfwWindow::key_callback(GLFWwindow *window, int key, int code, int action, int mode)
{
	if (key >= 0 && key < KEY_COUNT)
	{
		state.io.keyEvent[key] = true;
		if (action == GLFW_PRESS)
		{
			state.io.keyPressed[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			state.io.keyPressed[key] = false;
		}
	}
}

void GlfwWindow::mouse_callback(GLFWwindow *window, double x_pos, double y_pos)
{
	state.io.mouse.change += glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos)) - state.io.mouse.pos;
	state.io.mouse.pos     = glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos));
}

void GlfwWindow::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		state.io.mouse.rightEvent = true;
		if (action == GLFW_PRESS)
		{
			state.io.mouse.rightPressed = true;
		}
		else if (action == GLFW_RELEASE)
		{
			state.io.mouse.rightPressed = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		state.io.mouse.leftEvent = true;
		if (action == GLFW_PRESS)
		{
			state.io.mouse.leftPressed = true;
		}
		else if (action == GLFW_RELEASE)
		{
			state.io.mouse.leftPressed = false;
		}
	}
}

void GlfwWindow::mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	state.io.mouse.scrollChange += yoffset - state.io.mouse.scrollOffset;
	state.io.mouse.scrollOffset = yoffset;
}

}        // namespace vka