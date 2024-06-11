#include "ResourcePool.h"
#include <vka/resource_objects/Resource.h>
#include <vka/resource_objects/Image.h>
#include <vka/interface/commands/commands.h>
namespace vka
{
bool ResourcePool::add(Resource *resource)
{
	/*if (resource->hash() == 0)
	{
		printVka("ResourcePool::add() called with a resource that has a hash of 0\n");
		DEBUG_BREAK;
		return false;
	}*/

	return resources.insert(resource).second;
}
bool ResourcePool::add(Image_I *img)
{
	return images.insert(img).second;
}

bool ResourcePool::remove(Resource *resource)
{
	return resources.erase(resource);
}

bool ResourcePool::remove(Image_I *img)
{
	return images.erase(img);
}

void ResourcePool::refreshImages(CmdBuffer_I* cmdBuf)
{
	for (auto it = images.begin(); it != images.end(); ++it)
	{
		(*it)->changeExtent({gState.io.extent.width, gState.io.extent.height, 1});
		(*it)->recreate();
		vkaCmdTransitionLayout(cmdBuf, (*it), (*it)->getInitialLayout());
	}
}

void ResourcePool::clear()
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	resources.clear();
	for (auto it = images.begin(); it != images.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	images.clear();
}
}