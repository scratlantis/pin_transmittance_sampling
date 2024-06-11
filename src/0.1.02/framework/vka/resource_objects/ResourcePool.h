#pragma once
#include <unordered_set>
#include <vka/core/macros/misc.h>
#include <vka/core/functions/misc.h>
#include "Resource.h"
namespace vka
{
class Image_I;
class CmdBuffer_I;
class ResourcePool : public IResourcePool
{
  private:
	std::unordered_set<Resource*> resources;
	std::unordered_set<Image_I*> images;

  public:
	bool add(Resource *resource) override;
	bool add(Image_I *img) override;
	bool remove(Resource *resource) override;
	bool remove(Image_I *img) override;
	void refreshImages(CmdBuffer_I *cmdBuf) override;
	ResourcePool(){};
	~ResourcePool(){};
	void clear() override;
	DELETE_COPY_CONSTRUCTORS(ResourcePool);
};
}

