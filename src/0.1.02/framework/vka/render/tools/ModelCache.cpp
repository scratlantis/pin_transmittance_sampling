#include "ModelCache.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <vka/interface/commands/commands.h>
namespace vka
{


bool loadObj(std::string path, std::vector<ObjVertex> &vertexList, std::vector<Index> &indexList)
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
		DEBUG_BREAK
		return false;
	}
	std::unordered_map<ObjVertex, uint32_t> vertexMap;
	vertexMap.reserve(vertexAttributes.vertices.size());
	vertexList.reserve(vertexAttributes.vertices.size());
	uint32_t indexCount = 0;
	for (auto &shape : shapes)
	{
		indexCount +=shape.mesh.indices.size();
	}
	indexList.reserve(vertexAttributes.vertices.size());
	uint64_t           cnt = 0;
	for (auto &shape : shapes)
	{
		for (auto &index : shape.mesh.indices)
		{
			ObjVertex objVertex{};
			objVertex.v =
			    glm::vec3(
			        vertexAttributes.vertices[index.vertex_index * 3],
			        vertexAttributes.vertices[index.vertex_index * 3 + 1],
			        vertexAttributes.vertices[index.vertex_index * 3 + 2]);
			if (index.texcoord_index >= 0)
				objVertex.vt =
					glm::vec2(
						vertexAttributes.texcoords[index.texcoord_index * 2],
						vertexAttributes.texcoords[index.texcoord_index * 2 + 1]);
			if (index.normal_index >= 0)
				objVertex.vn =
					glm::vec3(
						vertexAttributes.normals[index.normal_index * 3],
						vertexAttributes.normals[index.normal_index * 3 + 1],
						vertexAttributes.normals[index.normal_index * 3 + 2]);
			auto it = vertexMap.insert({objVertex, cnt});
			// New vertex
			if (it.second)
			{
				vertexList.push_back(objVertex);
				indexList.push_back(cnt++);
			}
			// Vertex already exists
			else
			{
				indexList.push_back(it.first->second);
			}
		}
	}
	vertexList.shrink_to_fit();
	indexList.shrink_to_fit();
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
ModelData ModelCache::fetch(VkaCommandBuffer cmdBuf, std::string path, void (*parse)(VkaBuffer vertexBuffer, const std::vector<ObjVertex> &vertexList, VkaBuffer indexBuffer, const std::vector<Index> &indexList))
{
	ModelKey key{path, parse};
	auto     it = map.find(key);
	if (it == map.end())
	{
		ModelData modelData{};
		modelData.vertexBuffer  = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		modelData.indexBuffer   = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		modelData.surfaceBuffer = vkaCreateBuffer(pPool);        // Only used cpu side
		modelData.surfaceCount  = 1; // Only one surface for now
		std::string fullPath    = modelPath + path;
		std::vector<ObjVertex> vertexList;
		std::vector<Index>     indexList;
		if (loadObj(fullPath, vertexList, indexList))
		{
			map.insert({key, modelData});
			parse(modelData.vertexBuffer, vertexList, modelData.indexBuffer, indexList);
			vkaCmdUpload(cmdBuf, modelData.vertexBuffer);
			vkaCmdUpload(cmdBuf, modelData.indexBuffer);
		}
		return modelData;
	}
	return it->second;
}
}		// namespace vka

