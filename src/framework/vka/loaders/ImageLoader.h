#pragma once
#include "../common.h"
#include "../core/macros/macros.h"
#include <stb_image.h>
namespace vka
{
class ImageLoader
{
  public:
	ImageLoader();
	ImageLoader(std::string imagePath);
	~ImageLoader();
	bool mapImage(std::string imageName, bool useAlpha);
	void unmapImage();


	int getWidth() { return width; }
	int getHeight() { return height; }
	int getChannels() { return channels; }
	void *getData()
	{
		return data;
	}
	size_t getDataSize() const
	{
		return width * height * 4 * quantisation;

	}

  private:
	bool        isImageMapped = false;
	int         width;
	int         height;
	int         channels;
	int 		quantisation;
	void       *data;
	std::string imagePath;
};


}