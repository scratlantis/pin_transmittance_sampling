#pragma once
#include <framework/vka/render_model/common.h>
#include "DataStructs.h"
#include <random>

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
		float                 coef = 0.3;
		for (size_t i = 0; i < count; i++)
		{
			gaussiansData[i].mean.x   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].mean.y   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].mean.z   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].variance = 0.5 * margin * unormDistribution(gen32);
		}
	}

	size_t size() const
	{
		return gaussiansData.size();
	}

	void upload(CmdBuffer& cmdBuf)
	{
		cmdBuf.uploadData(*this, 0, gaussiansData.data(), sizeof(Gaussian) * gaussiansData.size());
	}
	~GaussianBuffer();

  private:
	std::vector<Gaussian> gaussiansData;
};

class PinBuffer : public BufferVma
{
  public:
	PinBuffer(uint32_t count,
	               VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
	    BufferVma(&gState.heap, sizeof(Pin) * count, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
		std::mt19937                          gen32(42);
		std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);
		pins.resize(count);
		for (size_t i = 0; i < pins.size(); i++)
		{
			pins[i].theta.x = 2.0 * PI * unormDistribution(gen32);
			pins[i].theta.y = 2.0 * PI * unormDistribution(gen32);
			pins[i].phi.x   = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
			pins[i].phi.y   = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
		}
	}
	size_t size() const
	{
		return pins.size();
	}

	void upload(CmdBuffer &cmdBuf)
	{
		cmdBuf.uploadData(*this, 0, pins.data(), sizeof(Pin) * pins.size());
	}


	Buffer buildTransmittanceBuffer(UniversalCmdBuffer& cmdBuf, const GaussianBuffer* gaussianBuf)
	{
		Buffer               pinTransmittanceBuf = BufferVma(&gState.heap, sizeof(float) * size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		glm::uvec3           workGroupSize  = {128, 1, 1};
		glm::uvec3           resolution     = {pins.size(), 1, 1};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
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
		cmdBuf.pushDescriptors(0, *gaussianBuf, *this, pinTransmittanceBuf);
		cmdBuf.dispatch(workGroupCount);
		return pinTransmittanceBuf;
	}
	~PinBuffer();

  private:
	std::vector<Pin> pins;
};

class GridBuffer : public BufferVma
{
  public:
	GridBuffer(uint32_t resolution, uint32_t cellSize,
	           const PinBuffer *pinBuf, const GaussianBuffer* gaussiansBuf,
		VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
	    BufferVma(&gState.heap, sizeof(Pin) * cellSize * resolution * resolution * resolution, bufferUsageFlags), res(resolution), cellSize(cellSize), pinBuf(pinBuf)
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
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
		computeState.specialisationEntrySizes           = glm3VectorSizes();
		computeState.specializationData                 = getByteVector(workGroupSize);
		ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
		cmdBuf.bindPipeline(computePipeline);
		cmdBuf.pushDescriptors(0, *pinBuf, (Buffer) *this);
		cmdBuf.dispatch(workGroupCount);
	}
	~GridBuffer(){};
	virtual void          bind(UniversalCmdBuffer& cmdBuf, const MemoryBlock& params) const
	{
	}
  private:
	virtual void bindPipeline(CmdBuffer& cmdBuf) const
	{
	}
	uint32_t res;
	uint32_t cellSize;
	const PinBuffer *pinBuf;
};


class GaussianNN_M : public Material
{
  public:
	GaussianNN_M(
	    const DefaultRenderPass *defaultRenderPass,
	    const Buffer            *viewBuf,
	    const GaussianBuffer    *gaussianBuf,
	    const PinBuffer         *pinBuf,
	    const Buffer            *pinTransmittanceBuf) :
	    gaussianBuf(gaussianBuf), pinBuf(pinBuf){};
	~GaussianNN_M(){};
	const GaussianBuffer *gaussianBuf;
	const PinBuffer      *pinBuf;
	const Buffer         *pinTransmittanceBuf;

	virtual void bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const
	{
	}

  private:
	virtual void bindPipeline(CmdBuffer &cmdBuf) const
	{
	}
};


class Gaussian_M : public Material
{
  public:
	Gaussian_M(
	    const DefaultRenderPass *defaultRenderPass,
		const Buffer *viewBuf,
		const GaussianBuffer *gaussianBuf) :
	    gaussianBuf(gaussianBuf){};
	~Gaussian_M(){};

	const GaussianBuffer* gaussianBuf;
	virtual void          bind(UniversalCmdBuffer& cmdBuf, const MemoryBlock& params) const
	{
	}
  private:
	virtual void bindPipeline(CmdBuffer& cmdBuf) const
	{
	}
};


class GaussianNNGrid_M : public Material
{
  public:
	GaussianNNGrid_M(
	    const DefaultRenderPass *defaultRenderPass,
		const Buffer *viewBuf,
		const GaussianBuffer *gaussianBuf,
		const PinBuffer *pinBuf,
		const Buffer *pinTransmittanceBuf,
		const GridBuffer *gridBuffer) :
		gaussianBuf(gaussianBuf), pinBuf(pinBuf){};
	~GaussianNNGrid_M(){};
	const GaussianBuffer *gaussianBuf;
	const PinBuffer      *pinBuf;
	const Buffer         *pinTransmittanceBuf;
	const GridBuffer     *gridBuffer;

	virtual void bind(UniversalCmdBuffer &cmdBuf, const MemoryBlock &params) const
	{
	}

  private:
	virtual void bindPipeline(CmdBuffer &cmdBuf) const
	{
	}
};
