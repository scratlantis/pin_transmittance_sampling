#pragma once
#include "common.h"
#include "mock.h"
#include <glm/glm.hpp>

#define KEY_COUNT 1024

class Device
{
	VkDevice logical;
	VkPhysicalDevice physical;
	VkInstance       instance;

	VkQueue getQueue();
};


struct Mouse
{
	//glm::vec2 change;
	//glm::vec2 pos;
	bool      leftPressed;
	bool      leftEvent;
	bool      rightPressed;
	bool      rightEvent;
	double    scrollOffset;
	double    scrollChange;
};

class I_InputOutput
{
	//Window					 window;
	VkExtent2D               extent;
	VkSurfaceKHR             surface;
	VkFormat                 format;
	VkSwapchainKHR           vkSwapchain;
	uint32_t                 imageCount;
	std::vector<VkImageView> imageViews;
	Mouse                    mouse;
	bool                     keyPressed[KEY_COUNT];
	bool                     keyEvent[KEY_COUNT];
};




struct Frame
{
	VkSemaphore renderFinishedSemaphore;
	VkSemaphore imageAvailableSemaphore;
	VkFence     inFlightFence;
	uint32_t    frameIndex;
	//RessourceTracker stack;
	Frame           *next;
	Frame           *previous;
};

struct State
{
	Device              device;
	I_InputOutput       io;
	//RessourceTracker    heap;
	//RessourceTracker    cache;
	Frame              *frame;
	//MemAllocator        memAlloc;
	//DescriptorAllocator descAlloc;
	//QueryAllocator      queryAlloc;
	//CmdAllocator        cmdAlloc;
};