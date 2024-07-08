#define STB_IMAGE_IMPLEMENTATION
#include "TextureCache.h"
namespace vka
{
bool loadImg(std::string path, ImageData &imgData, bool useAlpha = true)
{
	std::string suffix = path.substr(path.find_last_of(".") + 1);
	for (auto &c : suffix)
		c = tolower(c);
	if (suffix == "png" || suffix == "jpg")
	{
		imgData.data         = stbi_load(path.c_str(), &imgData.width, &imgData.height, &imgData.channels, (useAlpha) ? STBI_rgb_alpha : STBI_rgb);
		imgData.quantisation = 1;
	}
	else if (suffix == "hdr")
	{
		imgData.data         = stbi_loadf(path.c_str(), &imgData.width, &imgData.height, &imgData.channels, (useAlpha) ? STBI_rgb_alpha : STBI_rgb);
		imgData.quantisation = 4;
	}
	else
	{
		std::cout << "Unsupported image format " << suffix << std::endl;
		imgData.data = nullptr;
		DEBUG_BREAK;
	}
	imgData.channels = (useAlpha) ? 4 : 3;        // civ
	if (imgData.data)
	{
		return true;
	}
	else
	{
		std::cout << "Failed to load texture file " << path << std::endl;
		DEBUG_BREAK;
		return false;
	}
}

void unloadImg(ImageData &imgData)
{
	stbi_image_free(imgData.data);
	imgData.data = nullptr;
}

void TextureCache::clear()
{
	for (auto &texture : map)
	{
		texture.second->garbageCollect();
	}
	map.clear();
}

VkaImage TextureCache::fetch(VkaCommandBuffer cmdBuf, std::string path, VkFormat format, VkImageUsageFlags usage, VkImageLayout layout)
{
	auto it = map.find(path);
	if (it == map.end())
	{
		ImageData imgData;
		VkaImage  img = nullptr;
		if (loadImg(texturePath + path, imgData))
		{
			printVka(("Loading texture " + path).c_str());
			img = vkaCreateImage(pPool, format, usage, VkExtent2D{static_cast<uint32_t>(imgData.width), static_cast<uint32_t>(imgData.height)});
			vkaCmdUploadImageData(cmdBuf, imgData.data, imgData.size(), img, layout);
			unloadImg(imgData);
			map.insert({path, img});
		}
		return img;
	}
	VKA_ASSERT(it->second->getFormat() == format);
	VKA_ASSERT((it->second->getUsage() & usage) == usage);
	vkaCmdTransitionLayout(cmdBuf, it->second, layout);
	return it->second;
}
}        // namespace vka