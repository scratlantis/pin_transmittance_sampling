//#pragma once
//#include "../combined_resources/CmdBuffer.h"
//#include "../core/utility/misc.h"
//#include "vertex_macros.h"
//namespace vka
//{
//
//typedef uint32_t Index;
//
//// Frontend stuff
//
//
//class Geometry
//{
//  public:
//	virtual void                     move(ResourceTracker *pNewTracker) = 0;
//	//virtual bool                     load(std::string path)             = 0;
//	virtual void                     upload(CmdBuffer &cmdBuf) const    = 0;
//	Geometry(){};
//	~Geometry(){};
//
//  private:
//};
//template <typename VertexType, typename SurfaceDescriptor>
//class GeometryMaterial_T : public Geometry
//{
//  public:
//	std::vector<VertexType> vertices;
//	std::vector<Index>      indices;
//	std::vector<SurfaceDescriptor> surfaces;
//	std::vector<uint32_t> vertexOffsets;
//	std::vector<uint32_t> vertexCounts;
//	std::vector<uint32_t> indexOffsets;
//	std::vector<uint32_t> indexCounts;
//	Buffer vertexBuffer;
//	Buffer indexBuffer;
//	GeometryMaterial_T(ResourceTracker *pTracker,
//		std::vector<VertexType> &vertices,
//	    std::vector<Index>      &indices) :
//	    vertices(vertices), indices(indices)
//	{
//		indexCounts   = {(uint32_t) indices.size()};
//		vertexCounts  = {(uint32_t) vertices.size()};
//		indexOffsets = {0};
//		vertexOffsets = {0};
//		surfaces = {};
//		vertexBuffer = BufferVma(pTracker, sizeof(VertexType) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//		indexBuffer   = BufferVma(pTracker, sizeof(Index) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
//	};
//	virtual void move(ResourceTracker *pNewTracker) override
//	{
//		vertexBuffer.move(pNewTracker);
//		indexBuffer.move(pNewTracker);
//	}
//	virtual void upload(CmdBuffer &cmdBuf) const override
//	{
//		cmdBuf.uploadData(vertexBuffer, 0, vertices.data(), vertices.size());
//		cmdBuf.uploadData(indexBuffer, 0, indices.data(), indices.size());
//	}
//
//  private:
//};
//template <typename VertexType>
//class Geometry_T : public Geometry
//{
//  public:
//	std::vector<VertexType>        vertices;
//	std::vector<Index>             indices;
//	std::vector<uint32_t>          vertexOffsets;
//	std::vector<uint32_t>          vertexCounts;
//	std::vector<uint32_t>          indexOffsets;
//	std::vector<uint32_t>          indexCounts;
//	Buffer                         vertexBuffer;
//	Buffer                         indexBuffer;
//	Geometry_T(ResourceTracker         *pTracker,
//	           std::vector<VertexType> &vertices,
//	           std::vector<Index>      &indices) :
//	    vertices(vertices), indices(indices)
//	{
//		indexCounts   = {(uint32_t) indices.size()};
//		vertexCounts  = {(uint32_t) vertices.size()};
//		indexOffsets  = {0};
//		vertexOffsets = {0};
//		vertexBuffer  = BufferVma(pTracker, sizeof(VertexType) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
//		indexBuffer   = BufferVma(pTracker, sizeof(Index) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
//	};
//	virtual void move(ResourceTracker *pNewTracker) override
//	{
//		vertexBuffer.move(pNewTracker);
//		indexBuffer.move(pNewTracker);
//	}
//	virtual void upload(CmdBuffer &cmdBuf) const override
//	{
//		cmdBuf.uploadData(vertexBuffer, 0, vertices.data(), vertices.size());
//		cmdBuf.uploadData(indexBuffer, 0, indices.data(), indices.size());
//	}
//
//  private:
//};
//
//// Backend stuff
//class DrawSurface;
//class Material
//{
//  public:
//	Material(){};
//	~Material(){};
//
//	virtual void bind(UniversalCmdBuffer &cmdBuf) const = 0;
//	virtual void move(ResourceTracker *pNewTracker) = 0;
//
//  private:
//	virtual void bindPipeline(CmdBuffer &cmdBuf) const = 0;
//	DELETE_COPY_CONSTRUCTORS(Material);
//};
//
//template<typename View>
//class Material_T : public Material
//{
//  public:
//	Material_T(){};
//	~Material_T(){};
//	virtual void load(UniversalCmdBuffer &cmdBuf, const View& view) = 0;
//};
//
//
//
//class DrawSurface
//{
//  public:
//	DrawSurface(){};
//	~DrawSurface(){};
//
//	virtual Material *getMaterial() const = 0;
//	virtual Buffer   *getVertexBuffer() const = 0;
//	virtual uint32_t  getVertexCount() const  = 0;
//	virtual uint32_t  getVertexOffset() const = 0;
//	virtual Buffer   *getIndexBuffer() const  = 0;
//	virtual uint32_t  getIndexCount() const   = 0;
//	virtual uint32_t  getIndexOffset() const  = 0;
//
//	virtual void setMaterial(Material *pMaterial) = 0;
//	virtual void setVertexBuffer(Buffer *pVertexBuffer) = 0;
//	virtual void setVertexCount(uint32_t vertexCount)   = 0;
//	virtual void setVertexOffset(uint32_t vertexOffset) = 0;
//	virtual void setIndexBuffer(Buffer *pIndexBuffer)   = 0;
//	virtual void setIndexCount(uint32_t indexCount)     = 0;
//	virtual void setIndexOffset(uint32_t indexOffset)   = 0;
//  private:
//};
//template <typename View, typename Params>
//class DrawSurface_T : public DrawSurface
//{
//  public:
//	DrawSurface_T(){};
//	~DrawSurface_T(){};
//
//	void load(UniversalCmdBuffer &cmdBuf, const View &view)
//	{
//		pMaterial->load(cmdBuf, view);
//	}
//
//	void pushParams(UniversalCmdBuffer &cmdBuf)
//	{
//		cmdBuf.pushConstants(0, sizeof(Params), &params);
//	}
//
//	Material_T<View> *getMaterial() const
//	{
//		return pMaterial;
//	}
//	Buffer *getVertexBuffer() const
//	{
//		return pVertexBuffer;
//	}
//	uint32_t getVertexCount() const
//	{
//		return vertexCount;
//	}
//	uint32_t getVertexOffset() const
//	{
//		return vertexOffset;
//	}
//	Buffer *getIndexBuffer() const
//	{
//		return pIndexBuffer;
//	}
//	uint32_t getIndexCount() const
//	{
//		return indexCount;
//	}
//	uint32_t getIndexOffset() const
//	{
//		return indexOffset;
//	}
//	Params getParams() const
//	{
//		return params;
//	}
//
//	void setMaterial(Material *pMaterial)
//	{
//		this->pMaterial = pMaterial;
//	}
//	void setVertexBuffer(Buffer *pVertexBuffer)
//	{
//		this->pVertexBuffer = pVertexBuffer;
//	}
//	void setVertexCount(uint32_t vertexCount)
//	{
//		this->vertexCount = vertexCount;
//	}
//	void setVertexOffset(uint32_t vertexOffset)
//	{
//		this->vertexOffset = vertexOffset;
//	}
//	void setIndexBuffer(Buffer *pIndexBuffer)
//	{
//		this->pIndexBuffer = pIndexBuffer;
//	}
//	void setIndexCount(uint32_t indexCount)
//	{
//		this->indexCount = indexCount;
//	}
//	void setIndexOffset(uint32_t indexOffset)
//	{
//		this->indexOffset = indexOffset;
//	}
//	void setParams(Params params)
//	{
//		this->params = params;
//	}
//  private:
//	Params            params;
//	Material_T<View> *pMaterial;
//	Buffer           *pVertexBuffer;
//	uint32_t          vertexCount;
//	uint32_t          vertexOffset;
//	Buffer           *pIndexBuffer;
//	uint32_t          indexCount;
//	uint32_t          indexOffset;
//};
//
//
//
//
//
//
//
//class Model
//{
//  public:
//	Model(){};
//	~Model(){};
//	//virtual void                     move(ResourceTracker *pNewTracker) = 0;
//	//virtual void                     upload(CmdBuffer &cmdBuf) = 0;
//	virtual std::vector<DrawSurface> getDrawSurf() const = 0;
//
//  private:
//};
//
//template <typename VertexType, typename SurfaceDescriptor>
//class MaterialModel_T : public Model
//{
//  public:
//	MaterialModel_T(){};
//	MaterialModel_T(GeometryMaterial_T<VertexType, SurfaceDescriptor> *pGeometry) :
//	    pGeometry(pGeometry){};
//	~MaterialModel_T(){};
//	GeometryMaterial_T<VertexType, SurfaceDescriptor> *pGeometry;
//	/*virtual void                               move(ResourceTracker *pNewTracker) = 0;
//	virtual void                               upload(CmdBuffer &cmdBuf)          = 0;*/
//	virtual std::vector<DrawSurface>           getDrawSurf() const                = 0;
//};
//
//template <typename VertexType>
//class Model_T : public Model
//{
//  public:
//	Model_T(){};
//	Model_T(Geometry_T<VertexType>* pGeometry) :
//	    pGeometry(pGeometry){};
//	~Model_T(){};
//	Geometry_T<VertexType> *pGeometry;
//	/*virtual void                               move(ResourceTracker *pNewTracker) = 0;
//	virtual void                               upload(CmdBuffer &cmdBuf)          = 0;*/
//	virtual std::vector<DrawSurface> getDrawSurf() const = 0;
//};
//
//class DrawSurfaceInstance
//{
//  public:
//	DrawSurfaceInstance(const DrawSurface* pDrawSurf, void *instanceData, size_t instanceDataSize) :
//	    pDrawSurf(pDrawSurf)
//	{
//		this->instanceData = getByteVector(instanceData, instanceDataSize);
//	}
//	DrawSurfaceInstance(){};
//	~DrawSurfaceInstance(){};
//
//	const DrawSurface* pDrawSurf;
//	std::vector<uint8_t>  getInstanceData() const
//	{
//		return instanceData;
//	}
//  private:
//	std::vector<uint8_t> instanceData;
//};
//
//
//class DrawCall
//{
//  public:
//	DrawCall(){};
//	DrawCall(const DrawSurfaceInstance drawInstance) :
//	    drawSurf(drawInstance.drawSurf), instanceCount(1)
//	{
//		instanceData = {drawInstance.getInstanceData()};
//	}
//	void addInstance(const DrawSurfaceInstance drawInstance)
//	{
//		std::vector<uint8_t> newDrawInstanceData = drawInstance.getInstanceData();
//		instanceData.insert(instanceData.end(), newDrawInstanceData.begin(), newDrawInstanceData.end());
//		instanceCount++;
//	}
//	const DrawSurface drawSurf;
//	uint32_t          instanceCount;
//	std::vector<uint8_t> instanceData;
//
//	void submit(UniversalCmdBuffer &cmdBuf, ResourceTracker* pStack)
//	{
//		Buffer instanceBuffer = cmdBuf.uploadData(instanceData.data(), instanceData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, pStack, pStack);
//		std::vector buffers        = {drawSurf.pVertexBuffer->buf, instanceBuffer.buf};
//		cmdBuf.bindVertexBuffers(buffers);
//		cmdBuf.bindIndexBuffer(*drawSurf.pIndexBuffer);
//		cmdBuf.drawIndexed(drawSurf.indexCount, instanceCount);
//	}
//  private:
//};
//
//
////template <typename InstanceData>
////class DrawSurfaceInstance_T
////{
////  public:
////	DrawSurfaceInstance(const DrawSurface *pDrawSurf, InstanceData instanceData) :
////	    pDrawSurf(pDrawSurf), instanceData(instanceData){};
////	DrawSurfaceInstance(){};
////	~DrawSurfaceInstance(){};
////	const DrawSurface   *pDrawSurf;
////	InstanceData        instanceData;
////
////  private:
////};
////
////
////template <typename InstanceData>
////class DrawCall_T
////{
////  public:
////	DrawCall(){};
////	DrawCall(const DrawSurfaceInstance drawInstance) :
////	    drawSurf(drawInstance.drawSurf), instanceData(drawInstance)
////	{
////		instanceData = {drawInstance.getInstanceData()};
////	}
////	void addInstance(const DrawSurfaceInstance drawInstance)
////	{
////		instanceData.push_back(drawInstance.instanceData);
////	}
////	const DrawSurface*    drawSurf;
////	std::vector<InstanceData> instanceData;
////
////	void submit(UniversalCmdBuffer &cmdBuf, ResourceTracker *pStack)
////	{
////		Buffer      instanceBuffer = cmdBuf.uploadData(instanceData.data(), instanceData.size() * sizeof(InstanceData), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, pStack, pStack);
////		std::vector buffers        = {drawSurf.pVertexBuffer->buf, instanceBuffer.buf};
////		cmdBuf.bindVertexBuffers(buffers);
////		cmdBuf.bindIndexBuffer(*drawSurf->getIndexBuffer());
////		cmdBuf.drawIndexed(drawSurf.getIndexCount(), instanceCount);
////	}
////
////  private:
////};
//
//
//
//
////template <typename InstanceData>
////class Instance_T
////{
////  public:
////	  const Model *pModel;
////	  const InstanceData instanceData;
////	Instance_T(){};
////	Instance_T(const Model *pModel, const InstanceData &instanceData) :
////	    pModel(pModel), instanceData(instanceData){}
////	~Instance_T(){};
////
////
////
////  private:
////};
//
//
//
//
//
//} // namespace vka
//
//
