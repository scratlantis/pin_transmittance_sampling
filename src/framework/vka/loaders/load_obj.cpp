#pragma once
#define TINYOBJLOADER_IMPLEMENTATION
#include "load_obj.h"

namespace vka
{
bool loadObj(std::string name, std::vector<PosVertex>& vertices, std::vector<Index>& indices)
{
	tinyobj::attrib_t                vertexAttributes;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      errorString;
	std::string                      warningString;
	

	std::string path = gModelPath + "/" + name + "/" + name + ".obj";

	vertices.clear();
	indices.clear();

	bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, path.c_str());

	ASSERT_TRUE(success);

	if (!success)
	{
		std::cerr << "Failed to load model: " << name << std::endl;
		return false;
	}

	uint32_t vertexCount = vertexAttributes.vertices.size() / 3;

	for (size_t i = 0; i < vertexCount; i++)
	{
		PosVertex vertex;
		vertex.pos = {vertexAttributes.vertices[i*3], vertexAttributes.vertices[i*3 + 1], vertexAttributes.vertices[i*3 + 2]};
		vertices.push_back(vertex);
	}

	for (auto& shape : shapes)
	{
		for (auto& index : shape.mesh.indices)
		{
			indices.push_back(index.vertex_index);
		}
	}
	return true;
}
}        // namespace vka
