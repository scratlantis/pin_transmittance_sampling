#pragma once
#include "../render_model/common.h"
#include "../render_model/vertex_macros.h"


//MAKE_VERTEX_2ARG(PosColorVertex,
//         glm::vec3, pos, VK_FORMAT_R32G32B32_SFLOAT,
//         glm::vec3, color, VK_FORMAT_R32G32B32_SFLOAT)

namespace vka
{
template <typename VertexType>
class DefaulModel : public Model_T<VertexType>
{
  public:
	DefaulModel(){};
	Material *pMaterial;
	uint32_t  sortKey;
	DefaulModel(Geometry_T<VertexType> *pGeometry, Material *pMaterial, uint32_t sortKey = 0)
        :
        Model_T<VertexType>(pGeometry),
        pMaterial(pMaterial),
		sortKey(sortKey)
    {
    };
	virtual std::vector<DrawSurface> getDrawSurf() const override
	{
		std::vector<DrawSurface> drawSurfs;
		uint32_t                 vertexOffset = 0;
		uint32_t                 indexOffset  = 0;
		for (size_t i = 0; i < pGeometry->vertexCounts.size(); i++)
		{
			DrawSurface drawSurf;
			drawSurf.pMaterial    = pMaterial;
			drawSurf.vertexBuffer = pGeometry->vertexBuffer;
			drawSurf.vertexCount  = pGeometry->vertexCounts[i];
			drawSurf.vertexOffset = pGeometry->vertexOffsets[i];
			drawSurf.indexBuffer  = pGeometry->indexBuffer;
			drawSurf.indexCount   = pGeometry->indexCounts[i];
			drawSurf.indexOffset  = pGeometry->indexOffsets[i];
			drawSurf.sortKey      = sortKey;
			drawSurfs.push_back(drawSurf);
		}
		return drawSurfs;
	}
};

class PosVertex
{
  public:
	PosVertex(){};
	PosVertex(glm::vec3 pos) : pos(pos) {}
	glm::vec3 pos;
	static VkVertexInputBindingDescription getBindingDescription(uint32_t bindingIdx)
	{
		return getVertexBindingDescription(sizeof(PosVertex), bindingIdx);
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding)
	{
		return vka::getAttributeDescriptions(binding, {VK_FORMAT_R32G32B32_SFLOAT}, {0});
	}
};

class Transform
{
  public:
	glm::mat4 mat;
	glm::mat4 invMat;
	Transform(glm::mat4 mat = glm::mat4(1.0)) : mat(mat), invMat(glm::inverse(mat)) {}
    static VkVertexInputBindingDescription getBindingDescription(uint32_t bindingIdx)
	{
		return getInstanceBindingDescription(sizeof(Transform), bindingIdx);
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding)
    {
		return vka::getAttributeDescriptions(
			binding,
		    {
			    VK_FORMAT_R32G32B32A32_SFLOAT,
			    VK_FORMAT_R32G32B32A32_SFLOAT,
			    VK_FORMAT_R32G32B32A32_SFLOAT,
			    VK_FORMAT_R32G32B32A32_SFLOAT,
			    VK_FORMAT_R32G32B32A32_SFLOAT,
			    VK_FORMAT_R32G32B32A32_SFLOAT,
			    VK_FORMAT_R32G32B32A32_SFLOAT,
			    VK_FORMAT_R32G32B32A32_SFLOAT
		    },
			{
				0,
			    sizeof(glm::vec4),
			    2 * sizeof(glm::vec4),
			    3 * sizeof(glm::vec4),
			    4 * sizeof(glm::vec4),
			    5 * sizeof(glm::vec4),
			    6 * sizeof(glm::vec4),
			    7 * sizeof(glm::vec4)
			});
    }
};
//std::vector<PosVertex> cCubeVertecies =
//    {
//        PosVertex(glm::vec3(-1.0, -1.0, 1.0)),
//        PosVertex(glm::vec3(1.0, -1.0, 1.0)),
//        PosVertex(glm::vec3(1.0, 1.0, 1.0)),
//        PosVertex(glm::vec3(-1.0, 1.0, 1.0)),
//        PosVertex(glm::vec3(-1.0, -1.0, -1.0)),
//        PosVertex(glm::vec3(1.0, -1.0, -1.0)),
//        PosVertex(glm::vec3(1.0, 1.0, -1.0)),
//        PosVertex(glm::vec3(-1.0, 1.0, -1.0))};

static const std::vector<PosVertex> cCubeVertecies =
    {
        PosVertex(glm::vec3(0.0, 0.0, 1.0)),
        PosVertex(glm::vec3(1.0, 0.0, 1.0)),
        PosVertex(glm::vec3(1.0, 1.0, 1.0)),
        PosVertex(glm::vec3(0.0, 1.0, 1.0)),
        PosVertex(glm::vec3(0.0, 0.0, 0.0)),
        PosVertex(glm::vec3(1.0, 0.0, 0.0)),
        PosVertex(glm::vec3(1.0, 1.0, 0.0)),
        PosVertex(glm::vec3(0.0, 1.0, 0.0))};
static const std::vector<Index> cCubeIndices =
    {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3};


static const std::vector<PosVertex> cTriangleVertecies =
    {
        PosVertex(glm::vec3(-1.0, -1.0, 1.0)),
        PosVertex(glm::vec3(1.0, -1.0, 1.0)),
        PosVertex(glm::vec3(1.0, 1.0, 1.0))
};
static const std::vector<Index> cTriangleIndices = {0, 1, 2};

}; // namespace vka