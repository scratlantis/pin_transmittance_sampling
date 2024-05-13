#pragma once
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
	virtual void move(ResourceTracker *pNewTracker) = 0;
	// virtual bool                     load(std::string path)             = 0;
	virtual void upload(CmdBuffer &cmdBuf) const = 0;
	Geometry(){};
	~Geometry(){};

  private:
};
template <typename VertexType, typename SurfaceDescriptor>
class GeometryMaterial_T : public Geometry
{
  public:
	std::vector<VertexType>        vertices;
	std::vector<Index>             indices;
	std::vector<SurfaceDescriptor> surfaces;
	std::vector<uint32_t>          vertexOffsets;
	std::vector<uint32_t>          vertexCounts;
	std::vector<uint32_t>          indexOffsets;
	std::vector<uint32_t>          indexCounts;
	Buffer                         vertexBuffer;
	Buffer                         indexBuffer;
	GeometryMaterial_T(ResourceTracker         *pTracker,
	                   std::vector<VertexType> &vertices,
	                   std::vector<Index>      &indices) :
	    vertices(vertices), indices(indices)
	{
		indexCounts   = {(uint32_t) indices.size()};
		vertexCounts  = {(uint32_t) vertices.size()};
		indexOffsets  = {0};
		vertexOffsets = {0};
		surfaces      = {};
		vertexBuffer  = BufferVma(pTracker, sizeof(VertexType) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
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
	std::vector<VertexType> vertices;
	std::vector<Index>      indices;
	std::vector<uint32_t>   vertexOffsets;
	std::vector<uint32_t>   vertexCounts;
	std::vector<uint32_t>   indexOffsets;
	std::vector<uint32_t>   indexCounts;
	Buffer                  vertexBuffer;
	Buffer                  indexBuffer;
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


class DrawSurface;
class Material;

class Model
{
  public:
	Model(){};
	~Model(){};
	// virtual void                     move(ResourceTracker *pNewTracker) = 0;
	// virtual void                     upload(CmdBuffer &cmdBuf) = 0;
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
	virtual std::vector<DrawSurface> getDrawSurf() const = 0;
};

template <typename VertexType>
class Model_T : public Model
{
  public:
	Model_T(){};
	Model_T(Geometry_T<VertexType> *pGeometry) :
	    pGeometry(pGeometry){};
	~Model_T(){};
	Geometry_T<VertexType> *pGeometry;
	/*virtual void                               move(ResourceTracker *pNewTracker) = 0;
	virtual void                               upload(CmdBuffer &cmdBuf)          = 0;*/
	virtual std::vector<DrawSurface> getDrawSurf() const = 0;
};





// Backend stuff
class Material
{
  public:
	Material(){};
	~Material(){};

	virtual void load(UniversalCmdBuffer &cmdBuf, const MemoryBlock &view){};
	virtual void bind(UniversalCmdBuffer &cmdBuf) const
	{
		bind(cmdBuf, MemoryBlock());
	}
	virtual void bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const = 0;
	virtual void move(ResourceTracker *pNewTracker){};

  private:
	DELETE_COPY_CONSTRUCTORS(Material);
};

class DrawSurface
{
  public:
	DrawSurface(){};
	~DrawSurface(){};
	MemoryBlock params;           // material params
	MemoryBlock view;             // view params (projection matrix etc.)
	Material   *pMaterial;        // material (pipepline, descriptor sets)
	Buffer      vertexBuffer;
	uint32_t    vertexCount;
	uint32_t    vertexOffset;
	Buffer      indexBuffer;
	uint32_t    indexCount;
	uint32_t    indexOffset;

  private:
};


class DrawSurfaceInstance
{
  public:
	DrawSurfaceInstance(const DrawSurface drawSurf, void *instanceData, size_t instanceDataSize) :
	    pDrawSurf(drawSurf)
	{
		this->instanceData = getByteVector(instanceData, instanceDataSize);
	}
	DrawSurfaceInstance(){};
	~DrawSurfaceInstance(){};

	const DrawSurface   pDrawSurf;
	std::vector<uint8_t> getInstanceData() const
	{
		return instanceData;
	}

  private:
	std::vector<uint8_t> instanceData;
};

class DrawCall
{
  public:
	DrawCall(){};
	DrawCall(std::vector<DrawSurfaceInstance> drawInstances):
		drawSurf(drawInstances[0].pDrawSurf), instanceCount(drawInstances.size())
	{
		for (const DrawSurfaceInstance &drawInstance : drawInstances)
		{
			std::vector<uint8_t> newDrawInstanceData = drawInstance.getInstanceData();
			instanceData.insert(instanceData.end(), newDrawInstanceData.begin(), newDrawInstanceData.end());
		}
	}
	DrawCall(const DrawSurfaceInstance drawInstance) :
	    drawSurf(drawInstance.pDrawSurf), instanceCount(1)
	{
		instanceData = {drawInstance.getInstanceData()};
	}
	void addInstance(const DrawSurfaceInstance drawInstance)
	{
		std::vector<uint8_t> newDrawInstanceData = drawInstance.getInstanceData();
		instanceData.insert(instanceData.end(), newDrawInstanceData.begin(), newDrawInstanceData.end());
		instanceCount++;
	}
	const DrawSurface    drawSurf;
	uint32_t             instanceCount;
	std::vector<uint8_t> instanceData;

	void submit(UniversalCmdBuffer &cmdBuf, ResourceTracker *pStack)
	{
		Buffer      instanceBuffer = cmdBuf.uploadData(instanceData.data(), instanceData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, pStack, pStack);
		std::vector<VkBuffer> buffers        = {drawSurf.vertexBuffer.buf, instanceBuffer.buf};
		cmdBuf.bindVertexBuffers(buffers);
		cmdBuf.bindIndexBuffer(drawSurf.indexBuffer);
		cmdBuf.drawIndexed(drawSurf.indexCount, instanceCount);
	}
  private:
};
}        // namespace vka
