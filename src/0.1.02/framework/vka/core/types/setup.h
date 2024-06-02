#pragma once
#include <vulkan/vulkan.h>
#include <vector>

struct QueueFamilyIndices
{
	int graphicsFamily     = -1;
	int presentationFamily = -1;
	int computeFamily      = -1;

	int graphicsFamilyCount     = 0;
	int presentationFamilyCount = 0;
	int computeFamilyCount      = 0;

	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0 && computeFamily >= 0;
	}
};

struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR        surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentationMode;
};