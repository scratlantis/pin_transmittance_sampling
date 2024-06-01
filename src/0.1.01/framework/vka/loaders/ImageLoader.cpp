#include "ImageLoader.h"
#include <stb_image.h>
namespace vka
{

ImageLoader::ImageLoader()
{
}

ImageLoader::ImageLoader(std::string imageDir) :
    imagePath(imageDir + "/")
{
}



ImageLoader::~ImageLoader()
{
}
bool ImageLoader::mapImage(std::string imageName, bool useAlpha = true)
{
	ASSERT_TRUE(isImageMapped == false);
	std::string suffix = imageName.substr(imageName.find_last_of(".") + 1);
	for (auto &c : suffix)
		c = tolower(c);
	if (suffix == "png" || suffix == "jpg")
	{
		data         = stbi_load((imagePath + imageName).c_str(), &width, &height, &channels, (useAlpha) ? STBI_rgb_alpha : STBI_rgb);
		quantisation = 1;
	}
	else if (suffix == "hdr")
	{
		data = stbi_loadf((imagePath + imageName).c_str(), &width, &height, &channels, (useAlpha) ? STBI_rgb_alpha : STBI_rgb);
		quantisation = 4;
	}
	else
	{
		std::cout << "Unsupported image format " << suffix << std::endl;
		isImageMapped = false;
		return isImageMapped;
	}
	if (data)
	{
		isImageMapped = true;
	}
	else
	{
		std::cout << "Failed to load texture file " << (imagePath + imageName) << std::endl;
		isImageMapped = false;
	}
	channels = (useAlpha) ? 4 : 3;
	return isImageMapped;
}
void ImageLoader::unmapImage()
{
	ASSERT_TRUE(isImageMapped == true);
	stbi_image_free(data);
	isImageMapped = false;
}
} // namespace vka