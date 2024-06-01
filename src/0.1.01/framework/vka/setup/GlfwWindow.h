#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "../global_state.h"
#include "../core/macros/macros.h"
//#include <imgui_impl_glfw.h>
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
	void         initGui();
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




}        // namespace vka