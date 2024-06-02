#pragma once
#include "Resource.h"
#include "PipelineLayout.h"
#include "Shader.h"
namespace vka
{

struct BlendMode
{
	VkBlendFactor srcFactor;
	VkBlendFactor dstFactor;
	VkBlendOp     blendOp;

	VkBlendFactor alphaSrcFactor;
	VkBlendFactor alphaDstFactor;
	VkBlendOp     alphaBlendOp;

	bool operator==(BlendMode right)
	{
		return (srcFactor == right.srcFactor) && (dstFactor == right.dstFactor) && (blendOp == right.blendOp);
	}
};

struct RasterizationPipelineState
{
  private:
	std::vector<VkPipelineShaderStageCreateInfo>	 stages;
  public:
	std::vector<ShaderDefinition>                    shaderDefinitions;

	std::vector<uint32_t>							 specialisationEntryCounts;
	std::vector<uint32_t>							 specialisationEntrySizes;
	std::vector<uint8_t>							 specialisationData;



	std::vector<VkVertexInputBindingDescription>     vertexBindingDescription;
	std::vector<VkVertexInputAttributeDescription>   vertexAttributeDescriptions;
	VkViewport                                       viewport;
	VkRect2D                                         scissor;
	VkPipelineViewportStateCreateInfo                viewportState;
	VkPipelineRasterizationStateCreateInfo           rasterizationState;
	VkPipelineMultisampleStateCreateInfo             multisampleState;
	VkPipelineDepthStencilStateCreateInfo            depthStencilState;
	std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
	std::vector<VkDynamicState>                      dynamicStates;
	PipelineLayoutDefinition						 layout;
	VkPipelineInputAssemblyStateCreateInfo           inputAssemblyState;


	VkPipelineVertexInputStateCreateInfo             vertexInputState;
	VkPipelineColorBlendStateCreateInfo              colorBlendState;
	VkPipelineDynamicStateCreateInfo                 dynamicState;

	bool _equals(RasterizationPipelineState const &other) const;

	bool operator==(const RasterizationPipelineState &other) const
	{
		return _equals(other);
	}

	hash_t hash() const;


	VkGraphicsPipelineCreateInfo buildPipelineCI(ResourceTracker *pTracker, VkRenderPass renderPass, uint32_t subpassIndex);


	RasterizationPipelineState();
	RasterizationPipelineState &addVertexBinding(VkVertexInputBindingDescription desc);
	RasterizationPipelineState &addVertexAttribute(VkVertexInputAttributeDescription desc);
	RasterizationPipelineState &addVertexAttribute(std::vector<VkVertexInputAttributeDescription> desc);

	// &setVertexAttributes(std::vector<VkVertexInputAttributeDescription> vertexAttributes);

	RasterizationPipelineState &setPrimitiveTopology(VkPrimitiveTopology topology);

	RasterizationPipelineState &setExtent(uint32_t width, uint32_t height);

	RasterizationPipelineState &setDescriptorLayout(DescriptorSetLayoutDefinition layout);

	RasterizationPipelineState &setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);

	RasterizationPipelineState &enableDepthTest(VkCompareOp compareOp, VkBool32 enableDepthWrite);

	RasterizationPipelineState &disableDepthTest();


	VkPipelineColorBlendAttachmentState getBlendAttachment(BlendMode blendMode);

	void addBlendMode(uint32_t attachmentCnt, BlendMode blendMode);
	void addBlendMode(BlendMode blendMode);

	void addShaderDefinitions(ShaderDefinition shaderDef);

	template <class... Args>
	void addShaderDefinitions(ShaderDefinition shaderDef, Args... args)
	{
		shaderDefinitions.push_back(shaderDef);
		addShaderDefinitions(args...);
	}

	template <class... Args>
	RasterizationPipelineState &setShaderDefinitions(Args... args)
	{
		shaderDefinitions.clear();
		addShaderDefinitions(args...);
		return *this;
	}

	template <typename... Args>
	void addVertexBinding(VkVertexInputBindingDescription desc, Args... args)
	{
		addVertexBinding(desc);
		addVertexBinding(args...);
	}
	template <typename... Args>
	RasterizationPipelineState &setVertexBinding(Args... args)
	{
		vertexBindingDescription.clear();
		addVertexBinding(args...);
		return *this;
	}

	template <typename... Args>
	void addVertexAttribute(VkVertexInputBindingDescription desc, Args... args)
	{
		addVertexAttribute(desc);
		addVertexAttribute(args...);
	}

	/*template <typename... Args>
	void addVertexAttributes(std::vector<VkVertexInputBindingDescription> desc, Args... args)
	{
		addVertexAttributes(desc);
		addVertexAttributes(args...);
	}

	template <typename... Args>
	RasterizationPipelineState &setVertexAttribute(Args... args)
	{
		vertexAttributeDescriptions.clear();
		addVertexAttribute(args...);
		return *this;
	}
	template <typename... Args>
	RasterizationPipelineState &setVertexAttributes(Args... args)
	{
		vertexAttributeDescriptions.clear();
		addVertexAttributes(args...);
		return *this;
	}*/

	template <typename... Args>
	void addBlendMode(BlendMode blendMode, Args... args)
	{
		blendAttachments.push_back(getBlendAttachment(blendMode));
		addBlendMode(args...);
	}
	//template <typename... Args>
	//void addBlendMode(uint32_t attachmentCnt, BlendMode blendMode, Args... args)
	//{
	//	assert(attachmentCnt > 1);
	//	blendAttachments.push_back(getBlendAttachment(blendMode));
	//	addBlendMode(attachmentCnt - 1, args...)
	//}
	template <typename... Args>
	RasterizationPipelineState &setBlendMode(Args... args)
	{
		blendAttachments.clear();
		addBlendMode(args...);
		return *this;
	}
};

class RasterizationPipeline : public UniqueResource<VkPipeline>
{
  protected:
	void free()
	{
		vkDestroyPipeline(gState.device.logical, handle, nullptr);
	}
	void buildHandle()
	{
		VkGraphicsPipelineCreateInfo ci = pipelineState.buildPipelineCI(pTracker, renderPass, subpassIndex);
		VK_CHECK(vkCreateGraphicsPipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
	}
	//virtual bool _equals(RasterizationPipeline const &other) const
	//{
	//	return this->pipelineState == other.pipelineState
	//		&& this->renderPass == other.renderPass
	//		&& this->subpassIndex == other.subpassIndex;
	//}


	virtual bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		auto that = static_cast<RasterizationPipeline const &>(other);
		return *this == that;
	}


	RasterizationPipeline *copyToHeap() const
	{
		return new RasterizationPipeline(pTracker, pipelineState, renderPass, subpassIndex);
	}

  public:
	bool operator==(const RasterizationPipeline &other) const
	{
		return this->pipelineState == other.pipelineState && this->renderPass == other.renderPass && this->subpassIndex == other.subpassIndex;
	}
	hash_t _hash() const
	{
		hash_t hash = 0;
		hashCombine(hash, pipelineState.hash());
		hashCombine(hash, renderPass);
		hashCombine(hash, subpassIndex);
		return hash;
	};
	RasterizationPipeline(ResourceTracker *pTracker, const RasterizationPipelineState pipelineState, VkRenderPass renderPass, uint32_t subpassIndex);
	~RasterizationPipeline();

	const VkRenderPass               renderPass;
	const uint32_t                   subpassIndex;
	RasterizationPipelineState       getState() const
	{
		return pipelineState;
	}
  private:
	RasterizationPipelineState pipelineState;
};

}