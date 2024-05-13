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

using namespace vka;
#ifndef SHADER_DIR
#	define SHADER_DIR ""
#endif        // !TARGET_NAME     // !TARGET_NAME

using namespace vka;
#ifndef APP_NAME
#	define APP_NAME ""
#endif        // !TARGET_NAME     // !TARGET_NAME

GVar                gvar_use_pins{"use pins", true, GVAR_BOOL, GVAR_APPLICATION};
std::vector<GVar *> gVars{
    &gvar_use_pins};
AppState          gState;

const std::string gShaderPath = SHADER_DIR;


int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 400, 400);
	Window       *window   = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Camera initialization
	Camera camera = Camera(CameraCI_Default());
	// ImGui initialization
	ImGuiWrapper imguiWrapper = ImGuiWrapper();
	imguiWrapper.init();


	// Resources
	Buffer                viewBuf     = BufferVma(&gState.heap, sizeof(View), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	GaussianBuffer        gaussianBuf = GaussianBuffer(GAUSSIAN_COUNT, GAUSSIAN_MARGIN);
	PinBuffer             pinBuf      = PinBuffer(PIN_COUNT);
	GridBuffer            pinGridBuf  = GridBuffer(PIN_GRID_SIZE, PINS_PER_GRID_CELL, &pinBuf, &gaussianBuf);
	Buffer                pinTransmittanceBuf;
	DefaultRenderPass     renderPass = DefaultRenderPass();
	Geometry_T<PosVertex> cubeGeom   = Geometry_T<PosVertex>(&gState.heap, cCubeVertecies, cCubeIndices);

	std::vector<Model *>     models;
	std::vector<Transform *> transforms;

	Gaussian_M             gaussianMat                 = Gaussian_M(&renderPass, &viewBuf, &gaussianBuf);
	DefaulModel<PosVertex> gaussianCube                = DefaulModel<PosVertex>(&cubeGeom, &gaussianMat);
	Transform              gaussianCubeTransform       = Transform(glm::translate(glm::mat4(1.0), glm::vec3(-1.0, 0.0, 0.0)));
	models.push_back(&gaussianCube);
	transforms.push_back(&gaussianCubeTransform);

	GaussianNN_M           gaussianNNMat               = GaussianNN_M(&renderPass, &viewBuf, &gaussianBuf, &pinBuf, &pinTransmittanceBuf);
	DefaulModel<PosVertex> gaussianNNCube              = DefaulModel<PosVertex>(&cubeGeom, &gaussianNNMat);
	Transform              gaussianNNCubeTransform     = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)));
	models.push_back(&gaussianNNCube);
	transforms.push_back(&gaussianNNCubeTransform);

	GaussianNNGrid_M       gaussianNNGridMat           = GaussianNNGrid_M(&renderPass, &viewBuf, &gaussianBuf, &pinBuf, &pinTransmittanceBuf, &pinGridBuf);
	DefaulModel<PosVertex> gaussianNNGridCube          = DefaulModel<PosVertex>(&cubeGeom, &gaussianNNGridMat);
	Transform              gaussianNNGridCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(1.0, 0.0, 0.0)));


	models.push_back(&gaussianNNGridCube);
	transforms.push_back(&gaussianNNGridCubeTransform);

	// Create draw calls
	std::vector<DrawCall> drawCalls;
	for (size_t i = 0; i < models.size(); i++)
	{
		std::vector<DrawSurface>         drawSurf = models[i]->getDrawSurf();
		std::vector<DrawSurfaceInstance> drawSurfInst;
		for (size_t j = 0; j < drawSurf.size(); j++)
		{
			drawSurfInst.push_back(DrawSurfaceInstance(drawSurf[j], transforms[i], sizeof(Transform)));
		}
		drawCalls.push_back(DrawCall(drawSurfInst));
	}

	// Upload data
	UniversalCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	imguiWrapper.uploadResources(cmdBuf);
	cubeGeom.upload(cmdBuf);
	gaussianBuf.upload(cmdBuf);
	pinBuf.upload(cmdBuf);
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
		view.update(cnt, camera);

		UniversalCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		// Upload data
		cmdBuf.uploadData(viewBuf, 0, &view, sizeof(View));
		cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		// Render
		renderPass.beginRender(cmdBuf);
		for each (DrawCall dc in drawCalls)
		{
			dc.drawSurf.pMaterial->bind(cmdBuf);
			dc.submit(cmdBuf, &gState.frame->stack);
		}
		renderPass.endRender(cmdBuf);
		imguiWrapper.renderGui(cmdBuf);
		gState.swapBuffers(&cmdBuf);
	}
	// Cleanup
	vkDeviceWaitIdle(gState.device.logical);
	imguiWrapper.destroy();
	gState.destroy();
	delete window;
}