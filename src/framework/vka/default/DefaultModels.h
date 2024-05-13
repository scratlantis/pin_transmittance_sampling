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
	DefaulModel(Geometry_T<VertexType> *pGeometry, Material *pMaterial)
        :
        Model_T<VertexType>(pGeometry),
        pMaterial(pMaterial)
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
			drawSurfs.push_back(drawSurf);
		}
		return drawSurfs;
	}
};

ESACPE_NAMESPACE_VKA(
    MAKE_VERTEX_1ARG(PosVertex, glm::vec3, pos, VK_FORMAT_R32G32B32_SFLOAT)
)

class Transform
{
  public:
	Transform(glm::mat4 mat) : mat(mat), invMat(glm::inverse(mat)) {}
	glm::mat4 mat;
	glm::mat4 invMat;
    VkVertexInputBindingDescription getBindingDescription(uint32_t bindingIdx)
	{
		return getInstanceBindingDescription(sizeof(Transform), bindingIdx);
	}
	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
    {
        return vka::getAttributeDescriptions(
            VK_FORMAT_R32G32B32A32_SFLOAT, 0,
            VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4),
            VK_FORMAT_R32G32B32A32_SFLOAT, 2*sizeof(glm::vec4),
            VK_FORMAT_R32G32B32A32_SFLOAT, 3*sizeof(glm::vec4),
            VK_FORMAT_R32G32B32A32_SFLOAT, 4*sizeof(glm::vec4),
            VK_FORMAT_R32G32B32A32_SFLOAT, 5*sizeof(glm::vec4),
            VK_FORMAT_R32G32B32A32_SFLOAT, 6*sizeof(glm::vec4),
            VK_FORMAT_R32G32B32A32_SFLOAT, 7*sizeof(glm::vec4));
    }
};


std::vector<PosVertex> cCubeVertecies =
    {
        PosVertex(glm::vec3(-1.0, -1.0, 1.0)),
        PosVertex(glm::vec3(1.0, -1.0, 1.0)),
        PosVertex(glm::vec3(1.0, 1.0, 1.0)),
        PosVertex(glm::vec3(-1.0, 1.0, 1.0)),
        PosVertex(glm::vec3(-1.0, -1.0, -1.0)),
        PosVertex(glm::vec3(1.0, -1.0, -1.0)),
        PosVertex(glm::vec3(1.0, 1.0, -1.0)),
        PosVertex(glm::vec3(-1.0, 1.0, -1.0))};
std::vector<Index> cCubeIndices =
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


}; // namespace vka