#pragma once
#include "DataStructs.h"
#include <framework/vka/render_model/common.h>
#include <random>

BlendMode BLEND_MODE_NONE      = {VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
BlendMode BLEND_MODE_OVERWRITE = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD};
BlendMode BLEND_MODE_ADD       = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
BlendMode BLEND_MODE_SUBTRACT  = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_REVERSE_SUBTRACT};
BlendMode BLEND_MODE_ALPHA     = {VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};

using namespace vka;

class GaussianBuffer : public BufferVma
{
  public:
	GaussianBuffer(uint32_t count, float margin,
	               VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
	    BufferVma(&gState.heap, sizeof(Gaussian) * count, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
		std::mt19937                          gen32(42);
		std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);
		gaussiansData.resize(count);
		float coef = 0.3;
		for (size_t i = 0; i < count; i++)
		{
			gaussiansData[i].mean.x   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].mean.y   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].mean.z   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].variance = 0.5 * unormDistribution(gen32);
		}
	}

	size_t size() const
	{
		return gaussiansData.size();
	}

	void upload(CmdBuffer &cmdBuf)
	{
		cmdBuf.uploadData(gaussiansData.data(), sizeof(Gaussian) * gaussiansData.size(), *this);
	}
	~GaussianBuffer(){};

  private:
	std::vector<Gaussian> gaussiansData;
};

class PinBuffer : public BufferVma
{
  public:
	PinBuffer(uint32_t           count,
	          VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
	    BufferVma(&gState.heap, sizeof(Pin) * count, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
		std::mt19937                          gen32(42);
		std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);
		pins.resize(count);
		for (size_t i = 0; i < pins.size(); i++)
		{
			pins[i].phi.x   = 2.0 * PI * unormDistribution(gen32);
			pins[i].phi.y   = 2.0 * PI * unormDistribution(gen32);
			pins[i].theta.x = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
			pins[i].theta.y = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
		}
	}
	size_t size() const
	{
		return pins.size();
	}

	void upload(CmdBuffer &cmdBuf)
	{
		cmdBuf.uploadData(pins.data(), sizeof(Pin) * pins.size(), *this);
	}

	Buffer buildTransmittanceBuffer(UniversalCmdBuffer &cmdBuf, const GaussianBuffer *gaussianBuf)
	{
		Buffer               pinTransmittanceBuf = BufferVma(&gState.heap, sizeof(float) * size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		glm::uvec3           workGroupSize       = {128, 1, 1};
		glm::uvec3           resolution          = {pins.size(), 1, 1};
		glm::uvec3           workGroupCount      = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = "pins_eval_transmittance.comp";
		computeState.shaderDef.args.push_back({"GAUSSIAN_COUNT", std::to_string(GAUSSIAN_COUNT)});
		computeState.shaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
		computeState.specialisationEntrySizes           = glm3VectorSizes();
		computeState.specializationData                 = getByteVector(workGroupSize);
		ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
		cmdBuf.bindPipeline(computePipeline);
		cmdBuf.pushDescriptors(0, (Buffer) *gaussianBuf, (Buffer) * this, (Buffer) pinTransmittanceBuf);
		cmdBuf.dispatch(workGroupCount);
		return pinTransmittanceBuf;
	}

	Buffer buildDirectionBuffer(UniversalCmdBuffer &cmdBuf)
	{
		Buffer                 pinDirectionsBuf = BufferVma(&gState.heap, sizeof(glm::vec4) * size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		std::vector<glm::vec4> directions(size());
		for (size_t i = 0; i < size(); i++)
		{
			glm::vec2 x      = sin(pins[i].phi) * cos(pins[i].theta);
			glm::vec2 y      = sin(pins[i].phi) * sin(pins[i].theta);
			glm::vec2 z      = cos(pins[i].phi);
			glm::vec3 origin = glm::vec3(x.x, y.x, z.x) + glm::vec3(0.5);
			origin *= 0.866025403784;        // sqrt(3)/2
			glm::vec3 direction = glm::normalize(glm::vec3(x.y - x.x, y.y - y.x, z.y - z.x));
			directions[i]       = glm::vec4(direction, 0.0);
		}
		cmdBuf.uploadData(directions.data(), sizeof(glm::vec4) * directions.size(), pinDirectionsBuf, &gState.heap);
		return pinDirectionsBuf;
	}

	void writeLineBuffers(UniversalCmdBuffer &cmdBuf, Buffer &vertexBuf, Buffer &indexBuf)
	{
		std::vector<PosVertex> vertices(size() * 2);
		std::vector<Index>     indices(size() * 2);
		for (size_t i = 0; i < size(); i++)
		{
			glm::vec2 x      = sin(pins[i].phi) * cos(pins[i].theta);
			glm::vec2 y      = sin(pins[i].phi) * sin(pins[i].theta);
			glm::vec2 z      = cos(pins[i].phi);
			glm::vec3 origin = glm::vec3(x.x, y.x, z.x) + glm::vec3(0.5);
			origin *= 0.866025403784;        // sqrt(3)/2
			glm::vec3 direction = glm::normalize(glm::vec3(x.y - x.x, y.y - y.x, z.y - z.x));
			vertices[i * 2]     = PosVertex(origin + direction*0.1f);
			vertices[i * 2 + 1] = PosVertex(origin + direction * 0.866025403784f * 2.0f - direction * 0.1f);
			indices[i * 2]      = i * 2;
			indices[i * 2 + 1]  = i * 2 + 1;
		}
		cmdBuf.uploadData(vertices.data(), sizeof(PosVertex) * vertices.size(), vertexBuf, &gState.heap);
		cmdBuf.uploadData(indices.data(), sizeof(Index) * indices.size(), indexBuf, &gState.heap);
	}
	~PinBuffer(){};

  private:
	std::vector<Pin> pins;
};

class GridBuffer : public BufferVma
{
  public:
	GridBuffer(uint32_t resolution, uint32_t cellSize,
	           const PinBuffer *pinBuf, const GaussianBuffer *gaussiansBuf,
	           VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
	    BufferVma(&gState.heap, sizeof(PinGridEntry) * cellSize * resolution * resolution * resolution, bufferUsageFlags), res(resolution), cellSize(cellSize), pinBuf(pinBuf)
	{
	}
	size_t resolution() const
	{
		return res;
	}

	void build(UniversalCmdBuffer &cmdBuf)
	{
		glm::uvec3           workGroupSize  = {8, 8, 8};
		glm::uvec3           resolution     = {PIN_GRID_SIZE, PIN_GRID_SIZE, PIN_GRID_SIZE};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = "pins_grid_gen.comp";
		computeState.shaderDef.args.push_back({"PIN_GRID_SIZE", std::to_string(PIN_GRID_SIZE)});
		computeState.shaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		computeState.shaderDef.args.push_back({"PINS_PER_GRID_CELL", std::to_string(PINS_PER_GRID_CELL)});
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		// layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
		computeState.specialisationEntrySizes           = glm3VectorSizes();
		computeState.specializationData                 = getByteVector(workGroupSize);
		ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
		cmdBuf.bindPipeline(computePipeline);
		cmdBuf.pushDescriptors(0, (Buffer) *pinBuf, (Buffer) * this);
		cmdBuf.dispatch(workGroupCount);
	}
	~GridBuffer(){};

  private:
	uint32_t         res;
	uint32_t         cellSize;
	const PinBuffer *pinBuf;
};

class GaussianNN_M : public Material
{
  public:
	GaussianNN_M(
	    const DefaultRenderPass *defaultRenderPass,
	    const Buffer            *viewBuf,
	    const PinBuffer         *pinBuf,
	    const Buffer            *pinTransmittanceBuf,
	    const Buffer            *pinDirectionsBuffer,
	    const Buffer            *pinUsedBuffer) :
	    pinBuf(pinBuf), pinTransmittanceBuf(pinTransmittanceBuf), viewBuf(viewBuf),
		defaultRenderPass(defaultRenderPass), pinDirectionsBuffer(pinDirectionsBuffer), pinUsedBuffer(pinUsedBuffer){};
	~GaussianNN_M(){};
	const PinBuffer         *pinBuf;
	const Buffer            *pinTransmittanceBuf;
	const DefaultRenderPass *defaultRenderPass;
	const Buffer            *viewBuf;
	const Buffer            *pinDirectionsBuffer;
	const Buffer            *pinUsedBuffer;

	virtual void bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const
	{
		bindPipeline(cmdBuf);
		cmdBuf.pushDescriptors(0, (Buffer) *viewBuf, (Buffer) *pinBuf, (Buffer) *pinTransmittanceBuf, (Buffer) *pinDirectionsBuffer, (Buffer) *pinUsedBuffer);
	}

  private:
	virtual void bindPipeline(UniversalCmdBuffer &cmdBuf) const
	{
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		ShaderDefinition vertShaderDef{"pins_render.vert"};
		ShaderDefinition fragShaderDef{"pins_render_gaussian_nn.frag"};
		fragShaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		fragShaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT_SQRT)});
		vka::BlendMode             blendMode     = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
		RasterizationPipelineState pipelineState = RasterizationPipelineState();
		pipelineState
		    .addVertexAttribute(PosVertex::getAttributeDescriptions(0))
		    .addVertexAttribute(Transform::getAttributeDescriptions(1))
		    .setExtent(gState.io.extent.width, gState.io.extent.height)
		    .setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
		    .setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		    .setVertexBinding(PosVertex::getBindingDescription(0), Transform::getBindingDescription(1))
		    .setDescriptorLayout(layoutDefinition)
		    .setShaderDefinitions(vertShaderDef, fragShaderDef)
		    .enableDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true)
		    .setBlendMode(1, BLEND_MODE_OVERWRITE);
		RasterizationPipeline pipeline = defaultRenderPass->createPipeline(pipelineState, 0);
		cmdBuf.bindRasterizationPipeline(pipeline);
	}
};

class Gaussian_M : public Material
{
  public:
	Gaussian_M(
	    const DefaultRenderPass *defaultRenderPass,
	    const Buffer            *viewBuf,
	    const GaussianBuffer    *gaussianBuf) :
	    gaussianBuf(gaussianBuf), defaultRenderPass(defaultRenderPass), viewBuf(viewBuf){};
	~Gaussian_M(){};

	const GaussianBuffer    *gaussianBuf;
	const DefaultRenderPass *defaultRenderPass;
	const Buffer            *viewBuf;
	virtual void             bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const
	{
		bindPipeline(cmdBuf);
		cmdBuf.pushDescriptors(0, *viewBuf, *gaussianBuf);
	}

  private:
	virtual void bindPipeline(UniversalCmdBuffer &cmdBuf) const
	{
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		ShaderDefinition vertShaderDef{"pins_render.vert"};
		ShaderDefinition fragShaderDef{"pins_render_gaussian.frag"};
		fragShaderDef.args.push_back({"GAUSSIAN_COUNT", std::to_string(GAUSSIAN_COUNT)});
		vka::BlendMode             blendMode     = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
		RasterizationPipelineState pipelineState = RasterizationPipelineState();
		pipelineState
		    .addVertexAttribute(PosVertex::getAttributeDescriptions(0))
		    .addVertexAttribute(Transform::getAttributeDescriptions(1))
		    .setExtent(gState.io.extent.width, gState.io.extent.height)
		    .setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
		    .setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		    .setVertexBinding(PosVertex::getBindingDescription(0), Transform::getBindingDescription(1))
		    .setDescriptorLayout(layoutDefinition)
		    .setShaderDefinitions(vertShaderDef, fragShaderDef)
		    .enableDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true)
		    .setBlendMode(1, BLEND_MODE_OVERWRITE);
		RasterizationPipeline pipeline = defaultRenderPass->createPipeline(pipelineState, 0);
		cmdBuf.bindRasterizationPipeline(pipeline);
	}
};

class GaussianNNGrid_M : public Material
{
  public:
	GaussianNNGrid_M(
	    const DefaultRenderPass *defaultRenderPass,
	    const Buffer            *viewBuf,
	    const Buffer            *pinTransmittanceBuf,
	    const GridBuffer        *gridBuf) :
	    defaultRenderPass(defaultRenderPass), viewBuf(viewBuf), pinTransmittanceBuf(pinTransmittanceBuf), gridBuf(gridBuf){};
	~GaussianNNGrid_M(){};
	const Buffer            *pinTransmittanceBuf;
	const GridBuffer        *gridBuf;
	const DefaultRenderPass *defaultRenderPass;
	const Buffer            *viewBuf;

	virtual void bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const
	{
		bindPipeline(cmdBuf);
		cmdBuf.pushDescriptors(0, (Buffer) *viewBuf, (Buffer) *pinTransmittanceBuf, (Buffer) *gridBuf);
	}

  private:
	virtual void bindPipeline(UniversalCmdBuffer &cmdBuf) const
	{
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		ShaderDefinition vertShaderDef{"pins_render.vert"};
		ShaderDefinition fragShaderDef{"pins_render_gaussian_nn_grid.frag"};
		fragShaderDef.args.push_back({"PIN_GRID_SIZE", std::to_string(PIN_GRID_SIZE)});
		fragShaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		fragShaderDef.args.push_back({"PINS_PER_GRID_CELL", std::to_string(PINS_PER_GRID_CELL)});
		vka::BlendMode             blendMode     = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
		RasterizationPipelineState pipelineState = RasterizationPipelineState();
		pipelineState
		    .addVertexAttribute(PosVertex::getAttributeDescriptions(0))
		    .addVertexAttribute(Transform::getAttributeDescriptions(1))
		    .setExtent(gState.io.extent.width, gState.io.extent.height)
		    .setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
		    .setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		    .setVertexBinding(PosVertex::getBindingDescription(0), Transform::getBindingDescription(1))
		    .setDescriptorLayout(layoutDefinition)
		    .setShaderDefinitions(vertShaderDef, fragShaderDef)
		    .enableDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true)
		    .setBlendMode(1, BLEND_MODE_OVERWRITE);
		RasterizationPipeline pipeline = defaultRenderPass->createPipeline(pipelineState, 0);
		cmdBuf.bindRasterizationPipeline(pipeline);
	}
};

class Pins_M : public Material
{
   public:
	Pins_M(
	    const DefaultRenderPass *defaultRenderPass,
	    const Buffer            *viewBuf,
	    const Buffer            *pinUsedBuffer) :
	     viewBuf(viewBuf), defaultRenderPass(defaultRenderPass), pinUsedBuffer(pinUsedBuffer){};
	~Pins_M(){};
	const DefaultRenderPass *defaultRenderPass;
	const Buffer            *viewBuf;
	const Buffer            *pinUsedBuffer;
	virtual void             bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const
	{
		bindPipeline(cmdBuf);
		cmdBuf.pushDescriptors(0, (Buffer) *viewBuf, (Buffer) *pinUsedBuffer);
	}

   private:
	virtual void bindPipeline(UniversalCmdBuffer &cmdBuf) const
	{
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		ShaderDefinition           vertShaderDef{"pins_visualize.vert"};
		ShaderDefinition           fragShaderDef{"pins_visualize.frag"};
		fragShaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		vka::BlendMode             blendMode     = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
		RasterizationPipelineState pipelineState = RasterizationPipelineState();
		pipelineState
		    .addVertexAttribute(PosVertex::getAttributeDescriptions(0))
		    .addVertexAttribute(Transform::getAttributeDescriptions(1))
		    .setExtent(gState.io.extent.width, gState.io.extent.height)
		    .setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
		    .setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
		    .setVertexBinding(PosVertex::getBindingDescription(0), Transform::getBindingDescription(1))
		    .setDescriptorLayout(layoutDefinition)
		    .setShaderDefinitions(vertShaderDef, fragShaderDef)
		    .enableDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true)
		    .setBlendMode(1, BLEND_MODE_OVERWRITE);
		RasterizationPipeline pipeline = defaultRenderPass->createPipeline(pipelineState, 0);
		cmdBuf.bindRasterizationPipeline(pipeline);
	}
 };

 class WriteDepthBuffer_M : public Material
{
   public:
	WriteDepthBuffer_M(
	    const DefaultRenderPass *defaultRenderPass,
	    const Buffer            *viewBuf) :
	    viewBuf(viewBuf), defaultRenderPass(defaultRenderPass){};
	~WriteDepthBuffer_M(){};
	const Buffer            *viewBuf;
	const DefaultRenderPass *defaultRenderPass;
	virtual void             bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const
	{
		bindPipeline(cmdBuf);
		cmdBuf.pushDescriptors(0, (Buffer) *viewBuf);
	}

   private:
	virtual void bindPipeline(UniversalCmdBuffer &cmdBuf) const
	{
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		layoutDefinition.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		ShaderDefinition           vertShaderDef{"pins_render.vert"};
		ShaderDefinition           fragShaderDef{"write_depth_buffer.frag"};
		vka::BlendMode             blendMode     = {VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
		RasterizationPipelineState pipelineState = RasterizationPipelineState();
		pipelineState
		    .addVertexAttribute(PosVertex::getAttributeDescriptions(0))
		    .addVertexAttribute(Transform::getAttributeDescriptions(1))
		    .setExtent(gState.io.extent.width, gState.io.extent.height)
		    .setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
		    .setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		    .setVertexBinding(PosVertex::getBindingDescription(0), Transform::getBindingDescription(1))
		    .setDescriptorLayout(layoutDefinition)
		    .setShaderDefinitions(vertShaderDef, fragShaderDef)
		    .enableDepthTest(VK_COMPARE_OP_ALWAYS, true)
		    .setBlendMode(1, BLEND_MODE_NONE);
		RasterizationPipeline pipeline = defaultRenderPass->createPipeline(pipelineState, 0);
		cmdBuf.bindRasterizationPipeline(pipeline);
	}
 };
