#include "ModelCache.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <vka/interface/commands/commands.h>
namespace vka
{


bool loadObj(std::string path, VkaBuffer vertexBuffer, VkaBuffer indexBuffer, VkaBuffer surfaceBuffer, uint32_t &surfaceCount, uint32_t bytesPerVertex, void (*parse)(void *vertexPointer, uint32_t idx, const tinyobj::attrib_t &vertexAttributes) )
{
	tinyobj::attrib_t                vertexAttributes;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      errorString;
	std::string                      warningString;

	bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, path.c_str());

	if (!success)
	{
		std::cerr << "Failed to load model: " << path << std::endl;
		return false;
	}

	uint32_t vertexCount = vertexAttributes.vertices.size() / 3;
	void    *data        = vkaMapStageing(vertexBuffer, vertexCount * bytesPerVertex);
	for (size_t i = 0; i < vertexCount; i++)
	{
		parse(data, i, vertexAttributes);
		data = (void*)((uint8_t*)data + bytesPerVertex);
	}
	vkaUnmap(vertexBuffer);
	std::vector<Index> indices;
	surfaceCount = 0;
	for (auto &shape : shapes)
	{
		surfaceCount++;
		for (auto &index : shape.mesh.indices)
		{
			indices.push_back(index.vertex_index);
		}
	}
	vkaWriteStaging(indexBuffer, indices.data(), indices.size() * sizeof(Index));
	// Fix later
	SurfaceData surfaceData{};
	surfaceData.vertexOffset = 0;
	surfaceData.vertexCount = vertexCount;
	surfaceData.indexOffset = 0;
	surfaceData.indexCount = indices.size();
	vkaWriteStaging(indexBuffer, &surfaceData, sizeof(SurfaceData));
	return true;
}
void ModelCache::clear()
{
	for (auto &model : map)
	{
		if (model.second.vertexBuffer)
			model.second.vertexBuffer->garbageCollect();
		if (model.second.indexBuffer)
			model.second.indexBuffer->garbageCollect();
	}
	map.clear();
}
ModelData ModelCache::fetch(VkaCommandBuffer cmdBuf, std::string path, uint32_t bytesPerVertex, void (*parse)(void *vertexPointer, uint32_t idx, const tinyobj::attrib_t &vertexAttributes))
{
	auto it = map.find(path);
	if (it == map.end())
	{
		ModelData modelData;
		modelData.vertexBuffer = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		modelData.indexBuffer  = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		modelData.surfaceBuffer  = vkaCreateBuffer(pPool); // Only used cpu side
		modelData.surfaceCount = 0;
		if (loadObj(path, modelData.vertexBuffer, modelData.indexBuffer, modelData.surfaceBuffer, &modelData.surfaceCount, bytesPerVertex, parse))
		{
			map.insert({path, modelData});
		}
		vkaCmdUpload(cmdBuf, modelData.vertexBuffer);
		vkaCmdUpload(cmdBuf, modelData.indexBuffer);
		return modelData;
	}
	return it->second;
}



}		// namespace vka