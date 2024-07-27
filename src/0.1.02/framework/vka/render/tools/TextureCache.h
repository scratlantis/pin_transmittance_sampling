#pragma once
#include "stb_image.h"
#include <vka/interface/common.h>
namespace vka
{
struct ImageData
{
	int   width;
	int   height;
	int   channels;
	int   quantisation;
	void *data;

	size_t size() const
	{
		return width * height * channels * quantisation;
	}
};

class TextureCache
{
	std::unordered_map<std::string, VkaImage> map;
	std::string                               texturePath;
	IResourcePool                            *pPool;

  public:
	TextureCache(IResourcePool *pPool, std::string texturePath) :
	    texturePath(texturePath), pPool(pPool)
	{}

	void clear();
	VkaImage fetch(VkaCommandBuffer cmdBuf, std::string path, VkFormat format, VkImageUsageFlags usage, VkImageLayout layout);
};



} // namespace v