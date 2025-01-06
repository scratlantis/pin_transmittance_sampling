#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "vk_mem_alloc.h"

// clang-format off
#include "stdio.h"
#include <vka/global_state.h>
#include <vka/initializers/misc.h>
#include <vka/setup/GlfwWindow.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vka/combined_resources/AccelerationStructur.h>
#include <vka/combined_resources/Buffer.h>
#include <vka/combined_resources/CmdBuffer.h>
#include <vka/combined_resources/Image.h>
#include <vka/input/FreeCamera.h>
#include <vka/resources/ComputePipeline.h>
#include <vka/resources/DescriptorSetLayout.h>
#include <vka/resources/Shader.h>
#include <vka/gui/ImGuiWrapper.h>
#include <vka/default/DefaultModels.h>
#include <vka/default/DefaultRenderPass.h>
#include <random>
#include "DataStructs.h"
#include "materials.h"
#include <vka/compatibility.h>
#include <vka/input/FixedCamera.h>
#include <vka/loaders/ImageLoader.h>
#include <vka/experimental/EnvironmentMap.h>
// clang-format on
using namespace vka;
#ifndef SHADER_DIR
#	define SHADER_DIR ""
#endif        // !TARGET_NAME     // !TARGET_NAME

using namespace vka;
#ifndef APP_NAME
#	define APP_NAME ""
#endif        // !TARGET_NAME     // !TARGET_NAME

std::vector<GVar *> gVars{
    &gvar_use_pins,
    &gvar_pin_selection_coef,
	&gvar_use_gaus_blur,
	&gvar_use_exp_moving_average,
	&gvar_exp_moving_average_coef
};
AppState gState;

const std::string gShaderPath = SHADER_DIR;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gResourceBaseDir = RESOURCE_BASE_DIR;



int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	Window       *window   = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Camera initialization
	FreeCamera camera = FreeCamera(FreeCameraCI_Default());
	// ImGui initialization
	ImGuiWrapper imguiWrapper = ImGuiWrapper();
	imguiWrapper.init();

	// Resources

	FramebufferImage offscreenImage = FramebufferImage(&gState.heap,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent);
	FramebufferImage lastFrameImage = FramebufferImage(&gState.heap,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent);

	FramebufferImage pingPongImages[2] =
	    {
	        FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent),
	        FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent)
	};



	FramebufferImage depthImage     = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT, gState.io.extent);
	depthImage.createImageView(&gState.heap);
	Buffer         viewBuf     = BufferVma(&gState.heap, sizeof(View), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	GaussianBuffer gaussianBuf = GaussianBuffer(GAUSSIAN_COUNT, GAUSSIAN_MARGIN);
	PinBuffer      pinBuf      = PinBuffer(PIN_COUNT);
	Buffer         pinDirectionsBuffer;
	GridBuffer     pinGridBuf = GridBuffer(PIN_GRID_SIZE, PINS_PER_GRID_CELL, &pinBuf, &gaussianBuf);
	Buffer         pinTransmittanceBuf;
	Buffer            pinVertexBuffer = BufferVma(&gState.heap, sizeof(PosVertex) * 2 * PIN_COUNT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Buffer            pinIndexBuffer  = BufferVma(&gState.heap, sizeof(Index) * 2 * PIN_COUNT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Buffer            pinUsedBuffer   = BufferVma(&gState.heap, sizeof(uint32_t) * PIN_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	DefaultRenderPass renderPass = DefaultRenderPass(&depthImage, &offscreenImage);
	renderPass.init();

	Geometry_T<PosVertex> cubeGeom = Geometry_T<PosVertex>(&gState.heap, cCubeVertecies, cCubeIndices);
	Geometry_T<PosVertex> pinGeom  = Geometry_T<PosVertex>(&gState.heap, pinVertexBuffer, PIN_COUNT * 2, pinIndexBuffer, PIN_COUNT * 2);

	// Geometry_T<PosVertex> cubeGeom   = Geometry_T<PosVertex>(&gState.heap, cTriangleVertecies, cTriangleIndices);

	std::vector<Model *>     models;
	std::vector<Transform *> transforms;
	std::vector<uint32_t>    instanceCounts;

	Gaussian_M             gaussianMat           = Gaussian_M(&renderPass, &viewBuf, &gaussianBuf);
	DefaulModel<PosVertex> gaussianCube          = DefaulModel<PosVertex>(&cubeGeom, &gaussianMat);
	Transform              gaussianCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.0, 0.0)));
	models.push_back(&gaussianCube);
	transforms.push_back(&gaussianCubeTransform);
	instanceCounts.push_back(1);

	GaussianNN_M           gaussianNNMat           = GaussianNN_M(&renderPass, &viewBuf, &pinBuf, &pinTransmittanceBuf, &pinDirectionsBuffer, &pinUsedBuffer);
	DefaulModel<PosVertex> gaussianNNCube          = DefaulModel<PosVertex>(&cubeGeom, &gaussianNNMat);
	Transform              gaussianNNCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)));
	models.push_back(&gaussianNNCube);
	transforms.push_back(&gaussianNNCubeTransform);
	instanceCounts.push_back(PIN_COUNT_SQRT);

	GaussianNNGrid_M       gaussianNNGridMat           = GaussianNNGrid_M(&renderPass, &viewBuf, &pinTransmittanceBuf, &pinGridBuf);
	DefaulModel<PosVertex> gaussianNNGridCube          = DefaulModel<PosVertex>(&cubeGeom, &gaussianNNGridMat);
	Transform              gaussianNNGridCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(1.0, 0.0, 0.0)));
	models.push_back(&gaussianNNGridCube);
	transforms.push_back(&gaussianNNGridCubeTransform);
	instanceCounts.push_back(1);

	WriteDepthBuffer_M     fillCubeMat       = WriteDepthBuffer_M(&renderPass, &viewBuf);
	DefaulModel<PosVertex> fillCubeModel     = DefaulModel<PosVertex>(&cubeGeom, &fillCubeMat);
	Transform              fillCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)));
	models.push_back(&fillCubeModel);
	transforms.push_back(&fillCubeTransform);
	instanceCounts.push_back(1);

	Pins_M                 pinMat          = Pins_M(&renderPass, &viewBuf, &pinUsedBuffer);
	DefaulModel<PosVertex> pinMatModel     = DefaulModel<PosVertex>(&pinGeom, &pinMat);
	Transform              pinMatTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)));
	models.push_back(&pinMatModel);
	transforms.push_back(&pinMatTransform);
	instanceCounts.push_back(1);

	// std::vector<uint32_t> instanceCounts = {1, PIN_COUNT_SQRT, 1};

	// Upload data
	UniversalCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	imguiWrapper.uploadResources(cmdBuf);
	cubeGeom.upload(cmdBuf);
	gaussianBuf.upload(cmdBuf);
	pinBuf.upload(cmdBuf);
	cmdBuf.fillBuffer(pinUsedBuffer, 0);
	pinDirectionsBuffer = pinBuf.buildDirectionBuffer(cmdBuf);
	pinBuf.writeLineBuffers(cmdBuf, pinVertexBuffer, pinIndexBuffer);
	cmdBuf.transitionLayout(pingPongImages[0], VK_IMAGE_LAYOUT_GENERAL);
	cmdBuf.transitionLayout(pingPongImages[1], VK_IMAGE_LAYOUT_GENERAL);
	cmdBuf.transitionLayout(lastFrameImage, VK_IMAGE_LAYOUT_GENERAL);
	cmdBuf.transitionLayout(offscreenImage, VK_IMAGE_LAYOUT_GENERAL);

	EnvironmentMap envMap        = EnvironmentMap(&gState.heap,
	                                              ImageCreateInfo_Default(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, cResolution2k, VK_FORMAT_R32G32B32A32_SFLOAT),
	                                              ImageViewCreateInfo_Default(envMap.img, VK_FORMAT_R32G32B32A32_SFLOAT),
	                                              SamplerCreateInfo_Default(0.0));
	Sampler        envMapSampler = envMap.getSampler();
	std::string    envMapName    = std::string("envmap/2k/autumn_field_2k.hdr");
	ImageLoader imageLoader = ImageLoader(gResourceBaseDir + "/textures");
	// Upload image
	{
		imageLoader.mapImage(envMapName, true);
		cmdBuf.uploadImage(imageLoader.getData(), imageLoader.getDataSize(), envMap, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cmdBuf.imageMemoryBarrier(envMap);
		imageLoader.unmapImage();
	}

	cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	// Build buffers
	pinGridBuf.build(cmdBuf);
	pinTransmittanceBuf = pinBuf.buildTransmittanceBuffer(cmdBuf, &gaussianBuf);
	commitCmdBuffers(&cmdBuf, 1, &gState.frame->stack, gState.device.universalQueues[0]);
	vkDeviceWaitIdle(gState.device.logical);
	imguiWrapper.destroyStagingResources();

	// Main loop
	uint32_t  cnt = 0;
	View      view{};
	glm::mat4 lastViewMatrix = glm::mat4(1.0);
	//camera.mouse_control(0, 0);
	while (!gState.io.shouldTerminate())
	{
		imguiWrapper.newFrame();
		// Hot Reload
		if (gState.io.keyEvent[GLFW_KEY_R] && gState.io.keyPressed[GLFW_KEY_R])
		{
			vkDeviceWaitIdle(gState.device.logical);
			gState.cache.clear();
		}
		// Update cpu data
		camera.keyControl(0.016);
		if (gState.io.mouse.rightPressed)
		{
			camera.mouseControl(0.016);
		}
		view.update(cnt, camera);

		std::vector<Transform> finalTransforms(transforms.size());
		for (size_t i = 0; i < transforms.size(); i++)
		{
			glm::mat4 centerMat   = glm::translate(glm::mat4(1.0), glm::vec3(-0.5, -0.5, -0.5));
			glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0), glm::radians(cnt / 20.f), glm::vec3(0.0, 1.0, 0.0));
			// rotationMat     = glm::mat4(1.0);
			glm::mat4 scaleMat        = glm::scale(glm::mat4(1.0), glm::vec3(0.4, 0.4, 0.4));
			finalTransforms[i].mat    = transforms[i]->mat * rotationMat * scaleMat * centerMat;
			finalTransforms[i].invMat = glm::inverse(finalTransforms[i].mat);
		}
		// Create draw calls
		std::vector<DrawCall> drawCalls;
		for (size_t i = 0; i < models.size(); i++)
		{
			std::vector<DrawSurface>         drawSurf = models[i]->getDrawSurf();
			std::vector<DrawSurfaceInstance> drawSurfInst;
			for (size_t j = 0; j < drawSurf.size(); j++)
			{
				for (size_t k = 0; k < instanceCounts[i]; k++)
				{
					drawSurfInst.push_back(DrawSurfaceInstance(drawSurf[j], &finalTransforms[i], sizeof(Transform)));
				}
			}
			drawCalls.push_back(DrawCall(drawSurfInst));
		}

		UniversalCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		// Upload data
		cmdBuf.uploadData(&view, sizeof(View), viewBuf);
		cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		// Render
		for (size_t i = 0; i < drawCalls.size(); i++)
		{
			drawCalls[i].buildInstanceBuffer(cmdBuf, &gState.frame->stack);
		}

		cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDEX_READ_BIT);
		depthImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
		offscreenImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
		//vkCmdClearColorImage(vka_compatibility::getHandle(cmdBuf), offscreenImage, VK_IMAGE_LAYOUT_GENERAL, &gState.io.clearColor, 1, &gState.io.clearRange);
		envMap.render(cmdBuf, offscreenImage, viewBuf);
		cmdBuf.transitionLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		renderPass.beginRender(cmdBuf);
		for (size_t i = 0; i < drawCalls.size(); i++)
		{
			drawCalls[i].drawSurf.pMaterial->bind(cmdBuf);
			drawCalls[i].submit(cmdBuf);
		}
		renderPass.endRender(cmdBuf);
		// Post processing
		pingPongImages[0].update(&gState.heap, &gState.frame->stack, gState.io.extent);
		pingPongImages[1].update(&gState.heap, &gState.frame->stack, gState.io.extent);
		lastFrameImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
		cmdBuf.transitionLayout(pingPongImages[0], VK_IMAGE_LAYOUT_GENERAL);
		cmdBuf.transitionLayout(pingPongImages[1], VK_IMAGE_LAYOUT_GENERAL);
		cmdBuf.transitionLayout(lastFrameImage, VK_IMAGE_LAYOUT_GENERAL);

		//cmdBuf.barrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		if (gvar_use_gaus_blur.val.v_bool || gvar_use_exp_moving_average.val.v_bool)
		{
			cmdBuf.copyImage(offscreenImage, pingPongImages[0]);
			cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			{
				// Post processing
				glm::uvec3                    workGroupSize  = {16, 16, 1};
				glm::uvec3                    resolution     = {gState.io.extent.width, gState.io.extent.height, 1};
				glm::uvec3                    workGroupCount = getWorkGroupCount(workGroupSize, resolution);
				ComputePipelineState computeState{};
				computeState.specialisationEntrySizes           = glm3VectorSizes();
				computeState.specializationData                 = getByteVector(workGroupSize);
				
				if (gvar_use_gaus_blur.val.v_bool)
				{
					DescriptorSetLayoutDefinition layoutDefinition{};
					layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
					layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
					layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
					layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
					computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
					{
						// Gauss X
						computeState.shaderDef.path     = shaderPathPrefix + "gaussfilter1D.comp";
						computeState.shaderDef.args = {{"FILTER_X", ""}};
						ComputePipeline computePipeline = ComputePipeline(&gState.cache, computeState);
						cmdBuf.bindPipeline(computePipeline);
						cmdBuf.pushDescriptors(0, viewBuf, pingPongImages[0], pingPongImages[1]);
						cmdBuf.dispatch(workGroupCount);
					}
					cmdBuf.barrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
					{
						// Gauss Y
						computeState.shaderDef.path     = shaderPathPrefix + "gaussfilter1D.comp";
						computeState.shaderDef.args     = {{"FILTER_Y", ""}};
						ComputePipeline computePipeline = ComputePipeline(&gState.cache, computeState);
						cmdBuf.bindPipeline(computePipeline);
						cmdBuf.pushDescriptors(0, viewBuf, pingPongImages[1], pingPongImages[0]);
						cmdBuf.dispatch(workGroupCount);
					}
				}
				if (gvar_use_gaus_blur.val.v_bool && gvar_use_exp_moving_average.val.v_bool)
				{
					cmdBuf.barrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
				}
				if (gvar_use_exp_moving_average.val.v_bool)
				{
					DescriptorSetLayoutDefinition layoutDefinition{};
					layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
					layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
					layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
					layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
					layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
					computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
					// Exp moving average
					computeState.shaderDef.path     = shaderPathPrefix + "expMovingAverage.comp";
					ComputePipeline computePipeline = ComputePipeline(&gState.cache, computeState);
					cmdBuf.bindPipeline(computePipeline);
					cmdBuf.pushDescriptors(0, viewBuf, lastFrameImage, pingPongImages[0], pingPongImages[1]);
					cmdBuf.dispatch(workGroupCount);
				}
			}
			if (gvar_use_exp_moving_average.val.v_bool)
			{
				cmdBuf.copyImage(pingPongImages[1], offscreenImage);
				cmdBuf.copyImage(pingPongImages[1], lastFrameImage);
			}
			else
			{
				cmdBuf.copyImage(pingPongImages[0], offscreenImage);
			}
		}
		cmdBuf.copyToSwapchain(offscreenImage);
		imguiWrapper.renderGui(cmdBuf);
		gState.swapBuffers(&cmdBuf);
	}
	// Cleanup
	vkDeviceWaitIdle(gState.device.logical);
	renderPass.destroy();
	imguiWrapper.destroy();
	gState.destroy();
	delete window;
}