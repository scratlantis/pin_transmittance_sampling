#include "ModelCache.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <vka/core_interface/general_commands.h>
#include <vka/core_interface/buffer_utility.h>
namespace vka
{

bool loadObj(std::string path, std::vector<ObjVertex> &vertexList, std::vector<Index> &indexList, std::vector<uint32_t> &indexOffsets, uint32_t &totalIndexCount)
{
	tinyobj::attrib_t                vertexAttributes;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      errorString;
	std::string                      warningString;
	bool                             success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, path.c_str());

	if (!success)
	{
		std::cerr << "Failed to load model: " << path << std::endl;
		DEBUG_BREAK
		return false;
	}
	std::unordered_map<ObjVertex, uint32_t> vertexMap;
	vertexMap.reserve(vertexAttributes.vertices.size());
	vertexList.reserve(vertexAttributes.vertices.size());
	totalIndexCount       = 0;
	uint32_t surfaceCount = 0;
	for (auto &shape : shapes)
	{
		totalIndexCount += shape.mesh.indices.size();
		surfaceCount++;
	}
	indexList.reserve(totalIndexCount);
	indexOffsets.reserve(surfaceCount);
	uint64_t cnt = 0;
	for (auto &shape : shapes)
	{
		indexOffsets.push_back(indexList.size());
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

ModelData ModelCache::fetch(CmdBuffer cmdBuf, std::string path, void (*parse)(Buffer vertexBuffer, const std::vector<ObjVertex> &vertexList))
{
	ModelKey key{path, parse};
	auto     it = map.find(key);
	if (it == map.end())
	{
		ModelData modelData{};
		modelData.vertexBuffer          = createBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsageFlags);
		modelData.indexBuffer           = createBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsageFlags);
		std::string            fullPath = modelPath + path;
		std::vector<ObjVertex> vertexList;
		std::vector<Index>     indexList;
		if (loadObj(fullPath, vertexList, indexList, modelData.indexOffsets, modelData.indexCount))
		{
			map.insert({key, modelData});
			parse(modelData.vertexBuffer, vertexList);
			write(modelData.indexBuffer, indexList.data(), indexList.size()*sizeof(Index));
			cmdUpload(cmdBuf, modelData.vertexBuffer);
			cmdUpload(cmdBuf, modelData.indexBuffer);

		}
		return modelData;
	}
	return it->second;
}

}        // namespace vka
