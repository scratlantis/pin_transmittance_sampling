#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "stdio.h"
#include <framework/vka/global_state.h>
#include <framework/vka/initializers/misc.h>
#include <framework/vka/setup/GlfwWindow.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <framework/vka/combined_resources/AccelerationStructur.h>
#include <framework/vka/combined_resources/Buffer.h>
#include <framework/vka/combined_resources/CmdBuffer.h>
#include <framework/vka/combined_resources/Image.h>
#include <framework/vka/input/Camera.h>
#include <framework/vka/resources/ComputePipeline.h>
#include <framework/vka/resources/DescriptorSetLayout.h>
#include <framework/vka/resources/Shader.h>
#include <framework/vka/gui/ImGuiWrapper.h>
#include <framework/vka/default/DefaultModels.h>
#include <framework/vka/default/DefaultRenderPass.h>
#include <random>
#include "DataStructs.h"
#include "materials.h"
#include <framework/vka/compatibility.h>

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
	&gvar_pin_selection_coef
};
AppState          gState;

const std::string gShaderPath = SHADER_DIR;


int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	Window       *window   = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Camera initialization
	Camera camera = Camera(CameraCI_Default());
	// ImGui initialization
	ImGuiWrapper imguiWrapper = ImGuiWrapper();
	imguiWrapper.init();


	// Resources
	FramebufferImage offscreenImage = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, gState.io.format, gState.io.extent);
	FramebufferImage      depthImage     = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT, gState.io.extent);
	depthImage.createImageView(&gState.heap);
	Buffer                viewBuf     = BufferVma(&gState.heap, sizeof(View), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	GaussianBuffer        gaussianBuf = GaussianBuffer(GAUSSIAN_COUNT, GAUSSIAN_MARGIN);
	PinBuffer             pinBuf      = PinBuffer(PIN_COUNT);
	Buffer                pinDirectionsBuffer;
	GridBuffer            pinGridBuf  = GridBuffer(PIN_GRID_SIZE, PINS_PER_GRID_CELL, &pinBuf, &gaussianBuf);
	Buffer                pinTransmittanceBuf;
	DefaultRenderPass     renderPass = DefaultRenderPass(&depthImage);
	renderPass.init();
	Geometry_T<PosVertex> cubeGeom   = Geometry_T<PosVertex>(&gState.heap, cCubeVertecies, cCubeIndices);
	//Geometry_T<PosVertex> cubeGeom   = Geometry_T<PosVertex>(&gState.heap, cTriangleVertecies, cTriangleIndices);

	std::vector<Model *>     models;
	std::vector<Transform *> transforms;

	Gaussian_M             gaussianMat                 = Gaussian_M(&renderPass, &viewBuf, &gaussianBuf);
	DefaulModel<PosVertex> gaussianCube                = DefaulModel<PosVertex>(&cubeGeom, &gaussianMat);
	Transform              gaussianCubeTransform       = Transform(glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.0, 0.0)));
	models.push_back(&gaussianCube);
	transforms.push_back(&gaussianCubeTransform);

	GaussianNN_M           gaussianNNMat               = GaussianNN_M(&renderPass, &viewBuf, &pinBuf, &pinTransmittanceBuf, &pinDirectionsBuffer);
	DefaulModel<PosVertex> gaussianNNCube              = DefaulModel<PosVertex>(&cubeGeom, &gaussianNNMat);
	Transform              gaussianNNCubeTransform     = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)));
	models.push_back(&gaussianNNCube);
	transforms.push_back(&gaussianNNCubeTransform);

	GaussianNNGrid_M       gaussianNNGridMat           = GaussianNNGrid_M(&renderPass, &viewBuf, &pinTransmittanceBuf, &pinGridBuf);
	DefaulModel<PosVertex> gaussianNNGridCube          = DefaulModel<PosVertex>(&cubeGeom, &gaussianNNGridMat);
	Transform              gaussianNNGridCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(1.0, 0.0, 0.0)));


	models.push_back(&gaussianNNGridCube);
	transforms.push_back(&gaussianNNGridCubeTransform);
	std::vector<uint32_t> instanceCounts = {1, PIN_COUNT_SQRT, 1};
	

	// Upload data
	UniversalCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	imguiWrapper.uploadResources(cmdBuf);
	cubeGeom.upload(cmdBuf);
	gaussianBuf.upload(cmdBuf);
	pinBuf.upload(cmdBuf);
	pinDirectionsBuffer = pinBuf.buildDirectionBuffer(cmdBuf);

	cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	// Build buffers
	pinGridBuf.build(cmdBuf);
	pinTransmittanceBuf = pinBuf.buildTransmittanceBuffer(cmdBuf, &gaussianBuf);
	commitCmdBuffers(&cmdBuf, 1, &gState.frame->stack, gState.device.universalQueues[0]);
	vkDeviceWaitIdle(gState.device.logical);
	imguiWrapper.destroyStagingResources();

	// Main loop
	uint32_t cnt = 0;
	View     view{};
	glm::mat4 lastViewMatrix = glm::mat4(1.0);
	camera.mouse_control(0, 0);
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
		camera.key_control(gState.io.keyPressed, 0.016);
		if (gState.io.mouse.rightPressed)
		{
			camera.mouse_control(gState.io.mouse.change.x, gState.io.mouse.change.y);
		}
		view.update(cnt, camera);

		std::vector<Transform> finalTransforms(transforms.size());
		for (size_t i = 0; i < transforms.size(); i++)
		{
			glm::mat4 centerMat = glm::translate(glm::mat4(1.0), glm::vec3(-0.5, -0.5, -0.5));
			glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0), glm::radians(cnt / 20.f), glm::vec3(0.0, 1.0, 0.0));
			//rotationMat     = glm::mat4(1.0);
			glm::mat4 scaleMat = glm::scale(glm::mat4(1.0), glm::vec3(0.4, 0.4, 0.4));
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
		cmdBuf.transitionLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		renderPass.beginRender(cmdBuf);
		for (size_t i = 0; i < drawCalls.size(); i++)
		{
		    drawCalls[i].drawSurf.pMaterial->bind(cmdBuf);
		    drawCalls[i].submit(cmdBuf);

		}
		renderPass.endRender(cmdBuf);
		cmdBuf.barrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		if (gState.io.keyPressed[GLFW_KEY_E])
		{
			glm::uvec3           workGroupSize  = {16, 16, 1};
			glm::uvec3           resolution     = {gState.io.extent.width, gState.io.extent.height, 1};
			glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
			ComputePipelineState computeState{};
			computeState.shaderDef.name = "pins_render.comp";
			computeState.shaderDef.args.push_back({"GAUSSIAN_COUNT", std::to_string(GAUSSIAN_COUNT)});
			computeState.shaderDef.args.push_back({"PIN_GRID_SIZE", std::to_string(PIN_GRID_SIZE)});
			computeState.shaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
			computeState.shaderDef.args.push_back({"PINS_PER_GRID_CELL", std::to_string(PINS_PER_GRID_CELL)});
			DescriptorSetLayoutDefinition layoutDefinition{};
			layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
			layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
			layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
			computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
			computeState.specialisationEntrySizes           = glm3VectorSizes();
			computeState.specializationData                 = getByteVector(workGroupSize);
			ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
			// Record commands
			offscreenImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
			cmdBuf.transitionLayout(offscreenImage, VK_IMAGE_LAYOUT_GENERAL);
			cmdBuf.bindPipeline(computePipeline);
			cmdBuf.pushDescriptors(0, viewBuf, (Image) offscreenImage, pinTransmittanceBuf, pinGridBuf, gaussianBuf);
			cmdBuf.dispatch(workGroupCount);
			cmdBuf.copyToSwapchain(offscreenImage);
		}

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