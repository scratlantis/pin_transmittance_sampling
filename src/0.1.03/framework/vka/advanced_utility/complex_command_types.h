#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/core/core_utility/types.h>
#include <vka/core/resources/cachable/RasterizationPipeline.h>
#include <vka/core/resources/cachable/ComputePipeline.h>
#include <vka/core/resources/Descriptor.h>
namespace vka
{

class PipelineCmd
{
  protected:
	virtual RenderState getRenderState() const = 0;

  public:
	virtual void addDescriptor() const;
	virtual void addPushConstants() const;
	virtual void exec(CmdBuffer cmdBuf) const;
};
class DrawCmd : public PipelineCmd
{
  public:

	DrawCmd();

	DrawSurface                          surf;
	RasterizationPipelineDefinition		 pipelineDef;
	uint32_t                             instanceCount;
	VkRect2D_OP                          renderArea;

	void        exec(CmdBuffer cmdBuf) const;
  private:
	std::vector<Image>                   attachments;
	std::vector<ClearValue>              clearValues;
	std::vector<Buffer>                  instanceBuffers;
	std::vector<Descriptor>              descriptors;
	Buffer								 pushConstantsData;
	std::vector<uint32_t>                pushConstantsSizes;

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
	 std::vector<Descriptor>   descriptors;
	 glm::uvec3                workGroupCount;
	 Buffer                    pushConstantsData;
	 std::vector<uint32_t>     pushConstantsSizes;

	 RenderState getRenderState() const;
	 void        exec(CmdBuffer cmdBuf) const;
};

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
}        // namespace vka