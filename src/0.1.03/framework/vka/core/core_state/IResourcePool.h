#pragma once
namespace vka
{
class Resource;
class Image_R;
class CmdBuffer_R;
class IResourcePool
{
  public:
	virtual bool add(Resource *resource)            = 0;
	virtual bool add(Image_R *img)                  = 0;
	virtual bool remove(Resource *resource)         = 0;
	virtual bool remove(Image_R *img)               = 0;
	virtual void refreshImages(CmdBuffer_R *cmdBuf) = 0;
	virtual void clear()                            = 0;
};
}        // namespace vka