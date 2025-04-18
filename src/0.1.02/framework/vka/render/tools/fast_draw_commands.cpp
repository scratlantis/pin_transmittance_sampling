#include "fast_draw_commands.h"
#include "definition_build_tools.h"

void FastDrawState::drawRect(VkaCommandBuffer cmdBuf, VkaImage dst, glm::vec4 color, VkRect2D_OP area)
{
	DrawCmd drawCmd = {};
	setDefaults(drawCmd.pipelineDef);
	drawCmd.renderArea = area;
	drawCmd.model.surfaceData.vertexCount = 3;
	drawCmd.instanceCount = 1;
	addColorAttachment(drawCmd, dst, {}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_ALPHA, VKA_BLEND_OP_ALPHA);
	//addColorAttachment(drawCmd, dst, {0.2f, 0.2f, 0.2f, 0.0f}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
	createFramebuffer(drawCmd, *framebufferCache);
	drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
	drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
	drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;

	VkaBuffer ubo = vkaCreateBuffer(gState.frame->stack, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	vkaWriteStaging(ubo, static_cast<const void *>(&color), sizeof(glm::vec4));
	vkaCmdUpload(cmdBuf, ubo);
	addDescriptor(drawCmd, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

	addShader(drawCmd.pipelineDef, gVkaShaderPath + "fill.vert", {});
	addShader(drawCmd.pipelineDef, gVkaShaderPath + "fill.frag",{});
	vkaCmdDraw(cmdBuf, drawCmd);
}

void FastDrawState::drawEnvMap(VkaCommandBuffer cmdBuf, VkaImage dst, VkaImage envMap, SamplerDefinition samplerDef, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, VkRect2D_OP area)
{
	DrawCmd drawCmd = {};
	setDefaults(drawCmd.pipelineDef);
	drawCmd.renderArea                    = area;
	drawCmd.model.surfaceData.vertexCount = 3;
	drawCmd.instanceCount                 = 1;
	addColorAttachment(drawCmd, dst, {}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_ALPHA, VKA_BLEND_OP_ALPHA);
	// addColorAttachment(drawCmd, dst, {0.2f, 0.2f, 0.2f, 0.0f}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
	createFramebuffer(drawCmd, *framebufferCache);
	drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
	drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
	drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;

	struct PushDescriptor
	{
		glm::mat4 viewMat;
		glm::mat4 inverseViewMat;
		glm::mat4 projectionMat;
		glm::mat4 inverseProjectionMat;
	};

	VkaBuffer pdBuf = vkaCreateBuffer(gState.frame->stack, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	PushDescriptor *pd       = static_cast<PushDescriptor *>(vkaMapStageing(pdBuf, sizeof(PushDescriptor)));
	pd->viewMat             = viewMatrix;
	pd->inverseViewMat      = glm::inverse(viewMatrix);
	pd->projectionMat       = projectionMatrix;
	pd->inverseProjectionMat = glm::inverse(projectionMatrix);
	vkaUnmap(pdBuf);
	vkaCmdUpload(cmdBuf, pdBuf);
	addDescriptor(drawCmd, pdBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, &samplerDef, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

	addShader(drawCmd.pipelineDef, gVkaShaderPath + "fill.vert", {});
	addShader(drawCmd.pipelineDef, gVkaShaderPath + "envmap.frag", {});
	vkaCmdDraw(cmdBuf, drawCmd);
}

void FastDrawState::computeHistogram(VkaCommandBuffer cmdBuf, VkaImage src, SamplerDefinition *pSamplerDef, VkaBuffer dst, VkaBuffer average, VkRect2D_OP area)
{
	ComputeCmd computeCmd = {};
	setDefaults(computeCmd, area.extent, gVkaShaderPath + "histogram.comp");
	dst->changeSize(sizeof(uint32_t) * 256);
	dst->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	dst->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	dst->recreate();
	vkaCmdFillBuffer(cmdBuf, dst, 0, dst->getSize(), 0);

	average->changeSize(sizeof(float));
	average->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	average->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	average->recreate();
	vkaCmdFillBuffer(cmdBuf, average, 0.f, average->getSize(), 0);

	VkaBuffer ubo = vkaCreateBuffer(gState.frame->stack, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	vkaWriteStaging(ubo, static_cast<const void *>(&area), sizeof(VkRect2D_OP));
	vkaCmdUpload(cmdBuf, ubo);
	vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	addDescriptor(computeCmd, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	addDescriptor(computeCmd, dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	addDescriptor(computeCmd, pSamplerDef, VK_DESCRIPTOR_TYPE_SAMPLER);
	addDescriptor(computeCmd, src, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	addDescriptor(computeCmd, average, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	vkaCmdCompute(cmdBuf, computeCmd);

	vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	setDefaults(computeCmd, 1U, gVkaShaderPath + "histogram_sum.comp");
	addDescriptor(computeCmd, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	addDescriptor(computeCmd, dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	addDescriptor(computeCmd, average, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	vkaCmdCompute(cmdBuf, computeCmd);
}


void initStorageBuffer(VkaCommandBuffer cmdBuf, VkaBuffer buffer, VkDeviceSize size)
{
	buffer->changeSize(size);
	buffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	buffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	buffer->recreate();
	vkaCmdFillBuffer(cmdBuf, buffer, 0, buffer->getSize(), 0);
}

VkaBuffer makeUniformBuffer(VkaCommandBuffer cmdBuf, const void *data, VkDeviceSize size)
{
	VkaBuffer ubo = vkaCreateBuffer(gState.frame->stack, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	vkaWriteStaging(ubo, data, size);
	vkaCmdUpload(cmdBuf, ubo);
	return ubo;
}




void FastDrawState::normalize(VkaCommandBuffer cmdBuf, VkaBuffer buffer, uint32_t count)
{
	ComputeCmd computeCmd = {};
	setDefaults(computeCmd, count, gVkaShaderPath + "normalize.comp", {{"COUNT", std::to_string(count)}});
	addDescriptor(computeCmd, buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	vkaCmdCompute(cmdBuf, computeCmd);
}


void FastDrawState::marginalize(VkaCommandBuffer cmdBuf, VkaBuffer pdfHorizontal, VkaBuffer pdfVertical, VkaImage src, glm::uvec2 binCount)
{
	initStorageBuffer(cmdBuf, pdfHorizontal, sizeof(float) * binCount.x);
	initStorageBuffer(cmdBuf, pdfVertical, sizeof(float) * binCount.y);
	VkExtent2D        extent  = src->getExtent2D();
	VkaBuffer  ubo    = makeUniformBuffer(cmdBuf, &extent, sizeof(VkExtent2D));
	SamplerDefinition sampler = SamplerDefinition();

	vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	ComputeCmd computeCmd = {};
	setDefaults(computeCmd, extent.width, gVkaShaderPath + "marginalize.comp", {});
	addDescriptor(computeCmd, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	addDescriptor(computeCmd, pdfHorizontal, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	addDescriptor(computeCmd, &sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
	addDescriptor(computeCmd, src, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	vkaCmdCompute(cmdBuf, computeCmd);

	setDefaults(computeCmd, extent.height, gVkaShaderPath + "marginalize.comp", {{"SWAP_AXIS", ""}});
	addDescriptor(computeCmd, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	addDescriptor(computeCmd, pdfVertical, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	addDescriptor(computeCmd, &sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
	addDescriptor(computeCmd, src, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	vkaCmdCompute(cmdBuf, computeCmd);

	vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	normalize(cmdBuf, pdfHorizontal, binCount.x);
	normalize(cmdBuf, pdfVertical, binCount.y);
}



void FastDrawState::renderDistribution(VkaCommandBuffer cmdBuf, VkaBuffer src, uint32_t binCount, VkaImage dst, VkRect2D_OP area)
{
	DrawCmd drawCmd = {};
	setDefaults(drawCmd.pipelineDef);
	drawCmd.renderArea                    = area;
	drawCmd.model.surfaceData.vertexCount = 3;
	drawCmd.instanceCount                 = 1;
	addColorAttachment(drawCmd, dst, {}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_ALPHA, VKA_BLEND_OP_ALPHA);
	createFramebuffer(drawCmd, *framebufferCache);
	drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
	drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
	drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;

	VkaBuffer ubo = vkaCreateBuffer(gState.frame->stack, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	vkaWriteStaging(ubo, static_cast<const void *>(&area), sizeof(VkRect2D_OP));
	vkaCmdUpload(cmdBuf, ubo);
	addDescriptor(drawCmd, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, src, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

	addShader(drawCmd.pipelineDef, gVkaShaderPath + "fill.vert", {});
	addShader(drawCmd.pipelineDef, gVkaShaderPath + "render_distribution.frag", {{"BUCKETS", std::to_string(binCount)}});
	vkaCmdDraw(cmdBuf, drawCmd);
}



void FastDrawState::renderHistogram(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaBuffer average, VkaImage dst, VkRect2D_OP area)
{
	DrawCmd drawCmd = {};
	setDefaults(drawCmd.pipelineDef);
	drawCmd.renderArea                    = area;
	drawCmd.model.surfaceData.vertexCount = 3;
	drawCmd.instanceCount                 = 1;
	addColorAttachment(drawCmd, dst, {}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_ALPHA, VKA_BLEND_OP_ALPHA);
	createFramebuffer(drawCmd, *framebufferCache);
	drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
	drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
	drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;

	VkaBuffer ubo = makeUniformBuffer(cmdBuf, &area, sizeof(VkRect2D_OP));

	addDescriptor(drawCmd, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, src, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, average, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

	addShader(drawCmd.pipelineDef, gVkaShaderPath + "fill.vert", {});
	addShader(drawCmd.pipelineDef, gVkaShaderPath + "histogram_render.frag", {});
	vkaCmdDraw(cmdBuf, drawCmd);
}

void FastDrawState::accumulate(VkaCommandBuffer cmdBuf, VkaImage src, SamplerDefinition samplerDef, VkaImage dst, VkRect2D_OP area, uint32_t accumulationCount)
{

	struct PushDescriptor
	{
		glm::ivec2 offset;
		glm::uvec2 extent;
		glm::uint  count;
	};

	VkaBuffer       pdBuf    = vkaCreateBuffer(gState.frame->stack, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	PushDescriptor *pd       = static_cast<PushDescriptor *>(vkaMapStageing(pdBuf, sizeof(PushDescriptor)));
	pd->offset               = {area.offset.x, area.offset.y};
	pd->extent               = {area.extent.width, area.extent.height};
	pd->count                = accumulationCount;
	vkaUnmap(pdBuf);
	vkaCmdUpload(cmdBuf, pdBuf);

	DrawCmd drawCmd = {};
	setDefaults(drawCmd.pipelineDef);
	drawCmd.renderArea.extent                    = area.extent;
	drawCmd.model.surfaceData.vertexCount = 3;
	drawCmd.instanceCount                 = 1;
	addColorAttachment(drawCmd, dst, {}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_ALPHA, VKA_BLEND_OP_ALPHA);
	createFramebuffer(drawCmd, *framebufferCache);
	drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
	drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
	drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;

	addDescriptor(drawCmd, pdBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, &samplerDef, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, src, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

	addShader(drawCmd.pipelineDef, gVkaShaderPath + "fill.vert", {});
	addShader(drawCmd.pipelineDef, gVkaShaderPath + "accumulate.frag", {});
	vkaCmdDraw(cmdBuf, drawCmd);

}

void FastDrawState::renderSprite(VkaCommandBuffer cmdBuf, VkaImage src, SamplerDefinition samplerDef, VkaImage dst, VkRect2D_OP area)
{
	DrawCmd drawCmd = {};
	setDefaults(drawCmd.pipelineDef);
	drawCmd.renderArea                    = area;
	drawCmd.model.surfaceData.vertexCount = 3;
	drawCmd.instanceCount                 = 1;
	addColorAttachment(drawCmd, dst, {}, dst->getLayout(), dst->getLayout(), VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
	createFramebuffer(drawCmd, *framebufferCache);
	drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
	drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
	drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;

	addDescriptor(drawCmd, &samplerDef, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, src, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

	addShader(drawCmd.pipelineDef, gVkaShaderPath + "fill.vert", {});
	addShader(drawCmd.pipelineDef, gVkaShaderPath + "sprite.frag", {});
	vkaCmdDraw(cmdBuf, drawCmd);
}
