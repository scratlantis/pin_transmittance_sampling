#include "GlfwWindow.h"
#include "setup.h"


namespace vka
{
	GlfwWindow::GlfwWindow()
	{
		glfwInit()
	}
GlfwWindow::init(const WindowCI &windowCI, VkInstance &instance)
{
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// allow it to resize
	glfwWindowHint(GLFW_RESIZABLE, windowCI.resizable ? GLFW_TRUE : GLFW_FALSE);
	width  = windowCI.width;
	height = windowCI.height;
	window = glfwCreateWindow(width, height, windowCI.title.c_str(), NULL, NULL);
	ASSERT_TRUE(window);
	glfwGetFramebufferSize(window, &width, &height);
	glfwMakeContextCurrent(window);

	int cursorMode;
	switch (windowCI.cursorMode)
	{
		case VISIBLE:
			cursorMode = GLFW_CURSOR_NORMAL;
			break;
		case HIDDEN:
			cursorMode = GLFW_CURSOR_HIDDEN;
			break;
		case DISABLED:
			cursorMode = GLFW_CURSOR_DISABLED;
			break;
		default:
			cursorMode = GLFW_CURSOR_NORMAL;
			break;
	}

	glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
	glfwSetWindowUserPointer(window, this);

	mousePos                   = glm::vec2(0, 0);
	mouseLeftPressed           = false;
	mouseRightPressed          = false;
	mouseLeftPressedLastFrame  = false;
	mouseRightPressedLastFrame = false;
	memset(keyPressed, 0, sizeof(bool));
	memset(keyPressedLastFrame, 0, sizeof(bool));

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	ASSERT_VULKAN(glfwCreateWindowSurface(instance, window, nullptr, &surface));
}

VkSurfaceKHR GlfwWindow::getSurface() const
{
	return surface;
}

VkExtent2D GlfwWindow::size() const
{
	VkExtent2D extent{};
	extent.width  = width;
	extent.height = height;
	return extent;
}

void GlfwWindow::readInputs(Mouse &mouse, bool (&keyPressed)[KEY_COUNT], bool (&keyEvent)[KEY_COUNT])
{
	glfwPollEvents();
	mouse.leftPressed  = mouseLeftPressed;
	mouse.rightPressed = mouseRightPressed;
	mouse.pos          = mousePos;
	mouse.scrollOffset = scrollOffset;
	memcpy(keyPressed, keyPressed, 1024 * sizeof(bool));

	mouse.leftEvent    = mouseLeftPressed != mouseLeftPressedLastFrame;
	mouse.rightEvent   = mouseRightPressed != mouseRightPressedLastFrame;
	mouse.change       = mousePos - mousePosLastFrame;
	mouse.scrollChange = scrollOffset - scrollOffsetLastFrame;
	for (size_t i = 0; i < KEY_COUNT; i++)
	{
		keyEvent[i] = keyPressed[i] != keyPressedLastFrame[i];
	}

	mouseLeftPressedLastFrame  = mouseLeftPressed;
	mouseRightPressedLastFrame = mouseRightPressed;
	mousePosLastFrame          = mousePos;
	scrollOffsetLastFrame      = scrollOffset;
	memcpy(keyPressed, keyPressedLastFrame, 1024 * sizeof(bool));
}

bool GlfwWindow::shouldClose()
{
	return glfwWindowShouldClose(window);
}

void GlfwWindow::requestClose()
{
	glfwSetWindowShouldClose(window, GL_TRUE);
}

void GlfwWindow::changeSize(VkExtent2D newSize)
{
	glfwSetWindowSize(window, newSize.width, newSize.height);
}

void GlfwWindow::destroy()
{
	glfwDestroyWindow(window);
}

GlfwWindow::~GlfwWindow()
{
}
}        // namespace vka