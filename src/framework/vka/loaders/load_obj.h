#pragma once
#include "../common.h"
#include "tiny_obj_loader.h"
#include "../global_state.h"
#include "../render_model/common.h"

#include "../default/DefaultModels.h"

namespace vka
{


	//template <typename VertexType>
	//bool loadObj(std::string name, std::vector<VertexType> vertices, std::vector<Index> indices)
	//{
	//    tinyobj::attrib_t                vertexAttributes;
	//    std::vector<tinyobj::shape_t>    shapes;
	//    std::vector<tinyobj::material_t> materials;
	//    std::string                      errorString;
	//    std::string                      warningString;
	//
	//    std::vector<VertexType> vertices;
	//    std::vector<Index>   indices;
	//    //std::vector<Surface> surfaces;
	//
	//    std::vector<PointLight> pointLights;
	//
	//    bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, path, mtlBaseDir);
	//}

	bool loadObj(std::string name, std::vector<PosVertex>& vertices, std::vector<Index>& indices);
}