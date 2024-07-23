#pragma once
#include "Resource.h"
#include <unordered_set>
namespace vka
{
// Todo remove this
class Image_R;
class CmdBuffer_R;

class ResourcePool : public IResourcePool
{
  private:
	std::unordered_set<Resource *> resources;
	std::unordered_set<Image_R *>  images;

  public:
	bool add(Resource *resource) override;
	bool add(Image_R *img) override;
	bool remove(Resource *resource) override;
	bool remove(Image_R *img) override;
	void refreshImages(CmdBuffer_R *cmdBuf) override;
	ResourcePool(){};
	~ResourcePool(){};
	void clear() override;
	DELETE_COPY_CONSTRUCTORS(ResourcePool);
};
}        // namespace vka
