#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

// clang-format off
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
    &gvar_pin_selection_coef};
AppState gState;

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
	//FramebufferImage offscreenImage = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, gState.io.format, gState.io.extent);
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
	DefaultRenderPass renderPass = DefaultRenderPass(&depthImage);
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
		cmdBuf.transitionLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		renderPass.beginRender(cmdBuf);
		for (size_t i = 0; i < drawCalls.size(); i++)
		{
			drawCalls[i].drawSurf.pMaterial->bind(cmdBuf);
			drawCalls[i].submit(cmdBuf);
		}
		renderPass.endRender(cmdBuf);
		//cmdBuf.barrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
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