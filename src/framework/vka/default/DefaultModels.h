#pragma once
#include "../render_model/common.h"
#include "../render_model/vertex_macros.h"


//MAKE_VERTEX_2ARG(PosColorVertex,
//         glm::vec3, pos, VK_FORMAT_R32G32B32_SFLOAT,
//         glm::vec3, color, VK_FORMAT_R32G32B32_SFLOAT)

namespace vka
{
template <typename VertexType>
class DefaulModel : Model_T<VertexType>
{
  public:
	DefaulModel(){};
	Material *pMaterial;
	DefaulModel(Geometry_T<VertexType> *pGeometry, Material *pMaterial)
        :
        Model_T<VertexType>(pGeometry),
        pMaterial(pMaterial)
    {
        //this->pGeometry = pGeometry;
    };
	virtual std::vector<DrawSurface> getDrawSurf() const override
	{
		std::vector<DrawSurface> drawSurfs;
		uint32_t                 vertexOffset = 0;
		uint32_t                 indexOffset  = 0;
		for (size_t i = 0; i < pGeometry->vertexCounts.size(); i++)
		{
			DrawSurface drawSurf;
			drawSurf.pMaterial     = pMaterial;
			drawSurf.pVertexBuffer = &pGeometry->vertexBuffer;
			drawSurf.vertexCount   = pGeometry->vertexCounts[i];
			drawSurf.vertexOffset  = pGeometry->vertexOffsets[i];
			drawSurf.pIndexBuffer  = &pGeometry->indexBuffer;
			drawSurf.indexCount    = pGeometry->indexCounts[i];
			drawSurf.indexOffset   = pGeometry->indexOffsets[i];
			drawSurfs.push_back(drawSurf);
		}
		return drawSurfs;
	}
};

ESACPE_NAMESPACE_VKA(
    MAKE_VERTEX_1ARG(PosVertex, glm::vec3, pos, VK_FORMAT_R32G32B32_SFLOAT)
)

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