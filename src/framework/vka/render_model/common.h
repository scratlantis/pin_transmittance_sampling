#pragma once
#include "../combined_resources/CmdBuffer.h"
#include "../core/utility/misc.h"
#include "vertex_macros.h"
namespace vka
{

typedef uint32_t Index;

// Frontend stuff


class Geometry
{
  public:
	virtual void                     move(ResourceTracker *pNewTracker) = 0;
	//virtual bool                     load(std::string path)             = 0;
	virtual void                     upload(CmdBuffer &cmdBuf) const    = 0;
	Geometry(){};
	~Geometry(){};

  private:
};
template <typename VertexType, typename SurfaceDescriptor>
class GeometryMaterial_T : public Geometry
{
  public:
	std::vector<VertexType> vertices;
	std::vector<Index>      indices;
	std::vector<SurfaceDescriptor> surfaces;
	std::vector<uint32_t> vertexOffsets;
	std::vector<uint32_t> vertexCounts;
	std::vector<uint32_t> indexOffsets;
	std::vector<uint32_t> indexCounts;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	GeometryMaterial_T(ResourceTracker *pTracker,
		std::vector<VertexType> &vertices,
	    std::vector<Index>      &indices) :
	    vertices(vertices), indices(indices)
	{
		indexCounts   = {(uint32_t) indices.size()};
		vertexCounts  = {(uint32_t) vertices.size()};
		indexOffsets = {0};
		vertexOffsets = {0};
		surfaces = {};
		vertexBuffer = BufferVma(pTracker, sizeof(VertexType) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		indexBuffer   = BufferVma(pTracker, sizeof(Index) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	};
	virtual void move(ResourceTracker *pNewTracker) override
	{
		vertexBuffer.move(pNewTracker);
		indexBuffer.move(pNewTracker);
	}
	virtual void upload(CmdBuffer &cmdBuf) const override
	{
		cmdBuf.uploadData(vertexBuffer, 0, vertices.data(), vertices.size());
		cmdBuf.uploadData(indexBuffer, 0, indices.data(), indices.size());
	}

  private:
};
template <typename VertexType>
class Geometry_T : public Geometry
{
  public:
	std::vector<VertexType>        vertices;
	std::vector<Index>             indices;
	std::vector<uint32_t>          vertexOffsets;
	std::vector<uint32_t>          vertexCounts;
	std::vector<uint32_t>          indexOffsets;
	std::vector<uint32_t>          indexCounts;
	Buffer                         vertexBuffer;
	Buffer                         indexBuffer;
	Geometry_T(ResourceTracker         *pTracker,
	           std::vector<VertexType> &vertices,
	           std::vector<Index>      &indices) :
	    vertices(vertices), indices(indices)
	{
		indexCounts   = {(uint32_t) indices.size()};
		vertexCounts  = {(uint32_t) vertices.size()};
		indexOffsets  = {0};
		vertexOffsets = {0};
		vertexBuffer  = BufferVma(pTracker, sizeof(VertexType) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		indexBuffer   = BufferVma(pTracker, sizeof(Index) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	};
	virtual void move(ResourceTracker *pNewTracker) override
	{
		vertexBuffer.move(pNewTracker);
		indexBuffer.move(pNewTracker);
	}
	virtual void upload(CmdBuffer &cmdBuf) const override
	{
		cmdBuf.uploadData(vertexBuffer, 0, vertices.data(), vertices.size());
		cmdBuf.uploadData(indexBuffer, 0, indices.data(), indices.size());
	}

  private:
};

// Backend stuff
class Material
{
  public:
	Material(){};
	~Material(){};

	virtual void render(CmdBuffer &cmdBuf) const    = 0;
	virtual void move(ResourceTracker *pNewTracker) = 0;

  private:
	DELETE_COPY_CONSTRUCTORS(Material);
};

class DrawSurface
{
  public:
	DrawSurface(){};
	~DrawSurface(){};

	Material *pMaterial;
	Buffer   *pVertexBuffer;
	uint32_t  vertexCount;
	uint32_t  vertexOffset;
	Buffer   *pIndexBuffer;
	uint32_t  indexCount;
	uint32_t  indexOffset;

  private:
};

class Model
{
  public:
	Model(){};
	~Model(){};
	//virtual void                     move(ResourceTracker *pNewTracker) = 0;
	//virtual void                     upload(CmdBuffer &cmdBuf) = 0;
	virtual std::vector<DrawSurface> getDrawSurf() const = 0;

  private:
};

template <typename VertexType, typename SurfaceDescriptor>
class MaterialModel_T : public Model
{
  public:
	MaterialModel_T(){};
	MaterialModel_T(GeometryMaterial_T<VertexType, SurfaceDescriptor> *pGeometry) :
	    pGeometry(pGeometry){};
	~MaterialModel_T(){};
	GeometryMaterial_T<VertexType, SurfaceDescriptor> *pGeometry;
	/*virtual void                               move(ResourceTracker *pNewTracker) = 0;
	virtual void                               upload(CmdBuffer &cmdBuf)          = 0;*/
	virtual std::vector<DrawSurface>           getDrawSurf() const                = 0;
};

template <typename VertexType>
class Model_T : public Model
{
  public:
	Model_T(){};
	Model_T(Geometry_T<VertexType>* pGeometry) :
	    pGeometry(pGeometry){};
	~Model_T(){};
	Geometry_T<VertexType> *pGeometry;
	/*virtual void                               move(ResourceTracker *pNewTracker) = 0;
	virtual void                               upload(CmdBuffer &cmdBuf)          = 0;*/
	virtual std::vector<DrawSurface> getDrawSurf() const = 0;
};






class RenderEntity
{
  public:
	RenderEntity(const Model *pModel, glm::mat4 modelMat = glm::mat4(1.0)) :
	    pModel(pModel), modelMatrices({modelMat}){};
	//~RenderEntity(){};
	void getModelDrawSurfs()
	{
		pDrawSurfs = &pModel->getDrawSurf();
	}
	std::vector<glm::mat4>        modelMatrices;
	std::vector<DrawSurface> *pDrawSurfs;
  private:
	const Model                    *pModel;
};





} // namespace vka


