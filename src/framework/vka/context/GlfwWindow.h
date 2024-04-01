#pragma once
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "../global_state.h"
namespace vka
{


class GlfwWindow : public Window
{
  public:

	GlfwWindow();
	void init(const WindowCI &windowCI, VkInstance &instance);
	VkExtent2D   size() const;
	VkSurfaceKHR getSurface() const;
	void         readInputs(Mouse &mouse, bool (&keyPressed)[KEY_COUNT], bool (&keyEvent)[KEY_COUNT]);
	bool         shouldClose();
	void         requestClose();
	void         changeSize(VkExtent2D newSize);
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
}

void GlfwWindow::key_callback(GLFWwindow *window, int key, int code, int action, int mode)
{
	GlfwWindow *thisWindow = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
	if (key >= 0 && key < KEY_COUNT)
	{
		if (action == GLFW_PRESS)
		{
			thisWindow->keyPressed[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			thisWindow->keyPressed[key] = false;
		}
	}
}

void GlfwWindow::mouse_callback(GLFWwindow *window, double x_pos, double y_pos)
{
	GlfwWindow *thisWindow = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
	thisWindow->mousePos          = glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos));
}

void GlfwWindow::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	GlfwWindow *thisWindow = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
	bool              *mouseButton;
	switch (button)
	{
		case GLFW_MOUSE_BUTTON_RIGHT:
			mouseButton = &thisWindow->mouseRightPressed;
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			mouseButton = &thisWindow->mouseLeftPressed;
			break;
		default:
			mouseButton = nullptr;
			break;
	}
	if (mouseButton && action == GLFW_PRESS)
	{
		*mouseButton = true;
	}
	else if (mouseButton && action == GLFW_RELEASE)
	{
		*mouseButton = false;
	}
}

void GlfwWindow::mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	GlfwWindow *thisWindow = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
	thisWindow->scrollOffset      = yoffset;
}

}        // namespace vka