#pragma once
namespace vka
{
class Resource;
class Image_I;
class CmdBuffer_I;
class IResourcePool
{
  public:
	virtual bool add(Resource *resource) = 0;
	virtual bool add(Image_I *img)     = 0;
	virtual bool remove(Resource *resource) = 0;
	virtual bool remove(Image_I *img)       = 0;
	virtual void refreshImages(CmdBuffer_I* cmdBuf)       = 0;
	virtual void clear() = 0;
};
}        // namespace vka