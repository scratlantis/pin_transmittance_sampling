#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/core/core_utility/types.h>
#include <vka/core/resources/cachable/RasterizationPipeline.h>
#include <vka/core/resources/cachable/ComputePipeline.h>
#include <vka/core/resources/Descriptor.h>
namespace vka
{

struct RasterizationPipelineInitValues
{
	VkPipelineCreateFlags       flags;
	VkSampleCountFlagBits       sampleCount;
	float                       minSampleShading;
	VkPrimitiveTopology         primitiveTopology;
	float                       lineWidth;
	float                       minDepthBounds;
	float                       maxDepthBounds;
	VkLogicOp                   blendLogicOp;
	std::vector<VkDynamicState> dynamicStates;
};

struct BlendOperation
{
	VkBlendFactor srcFactor;
	VkBlendFactor dstFactor;
	VkBlendOp     op;

	static BlendOperation write()
	{
		return BlendOperation{VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD};
	}
	static BlendOperation alpha()
	{
		return BlendOperation{VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
	}
};


class PipelineCmd
{
  protected:

	std::vector<Descriptor>              descriptors;
	Buffer								 pushConstantsData = nullptr;
	std::vector<uint32_t>                pushConstantsSizes;
  public:
	virtual RenderState getRenderState() const = 0;
	virtual void exec(CmdBuffer cmdBuf) const = 0;
	PipelineCmd();

	void pushConstant(void* data, VkDeviceSize size);
};
class DrawCmd : public PipelineCmd
{
  public:

	DrawCmd();

	void pushDepthAttachment(Image depthImage, bool clear, VkBool32 enableWrite, VkCompareOp compareOp);
	void pushColorAttachment(Image image, ClearValue clearValue, VkImageLayout layoutIn, VkImageLayout layoutOut, BlendOperation colorBlendOp, BlendOperation alphaBlendOp);
	void pushColorAttachment(Image image, VkImageLayout layoutOut, ClearValue clearValue = {});
	void pushColorAttachment(Image image, ClearValue clearValue = {});


	void pushDescriptor(BufferRef buffer, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(Image image, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(const SamplerDefinition sampler, VkShaderStageFlags shaderStage);
	void pushDescriptor(const SamplerDefinition sampler, Image image, VkShaderStageFlags shaderStage);
	void pushDescriptor(std::vector<BufferRef> buffers, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(std::vector<Image> images, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(std::vector<SamplerDefinition> samplersDefs, VkShaderStageFlags shaderStage);
	void pushDescriptor(TLASRef as, VkShaderStageFlags shaderStage);

	void setGeometry(DrawSurface surface);
	void pushInstanceData(BufferRef buffer, VertexDataLayout layout);


	DrawSurface                     surf          = {};
	RasterizationPipelineDefinition pipelineDef   = {};
	uint32_t                        instanceCount = 1;
	VkRect2D_OP                     renderArea    = {};

	void        exec(CmdBuffer cmdBuf) const;
  private:
	std::vector<Image>      attachments;
	std::vector<ClearValue> clearValues;
	std::vector<BufferRef>  instanceBuffers;

	RenderState getRenderState() const; 
};

class ComputeCmd : public PipelineCmd
{
   public:
	 ComputeCmd() = default;
	 ComputeCmd(uint32_t taskSize, const std::string path, std::vector<ShaderArgs> args = {});
	 ComputeCmd(glm::uvec2 taskSize, std::string path, std::vector<ShaderArgs> args);
	 ComputeCmd(VkExtent2D taskSize, std::string path, std::vector<ShaderArgs> args);
	 ComputeCmd(glm::uvec3 taskSize, std::string path, std::vector<ShaderArgs> args);
	 ComputePipelineDefinition pipelineDef;

   private:
	 glm::uvec3                workGroupCount;

	 RenderState getRenderState() const;
	 void        exec(CmdBuffer cmdBuf) const;
	 void        pushDescriptor(BufferRef buffer, VkDescriptorType type);
	 void        pushDescriptor(Image image, VkDescriptorType type);
	 void        pushDescriptor(const SamplerDefinition sampler);
	 void        pushDescriptor(const SamplerDefinition sampler, Image image);
	 void		 pushDescriptor(std::vector<Image> images, VkDescriptorType type);
	 void        pushDescriptor(std::vector<BufferRef> buffers, VkDescriptorType type);
	 void        pushDescriptor(std::vector<SamplerDefinition> samplersDefs);
	 void        pushDescriptor(TLASRef as, VkShaderStageFlags shaderStage);
};


}        // namespace vka