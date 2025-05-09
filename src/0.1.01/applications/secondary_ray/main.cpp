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
#include <vka/input/FixedCamera.h>
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
#include <vka/loaders/load_obj.h>
#include <vka/default/ConfigurableRenderPass.h>
#include <vka/resources/Sampler.h>
#include <vka/loaders/ImageLoader.h>
#include <vka/experimental/EnvironmentMap.h>

// clang-format on
using namespace vka;
#ifndef SHADER_DIR
#	define SHADER_DIR ""
#endif

#ifndef MODEL_DIR
#	define MODEL_DIR ""
#endif

using namespace vka;
#ifndef APP_NAME
#	define APP_NAME ""
#endif        // !TARGET_NAME     // !TARGET_NAME

//#define POST_PROCESSING
std::vector<GVar *> gVars{
    &gvar_use_pins,
    &gvar_pin_selection_coef,
    &gvar_ray_lenght,
	&gvar_positional_jitter,
	&gvar_angular_jitter,
    &gvar_use_env_map
#ifdef POST_PROCESSING
	,
	&gvar_use_gaus_blur,
	&gvar_use_exp_moving_average,
	&gvar_exp_moving_average_coef
#endif        // POST_PROCESSING

};
AppState gState;

const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gResourceBaseDir  = RESOURCE_BASE_DIR;


enum RenderPassID
{
	RENDER_PASS_WIREFRAME,
	RENDER_PASS_VOLUME,
	RENDER_PASS_MATERIAL
};

int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	Window       *window   = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Camera initialization
	//FreeCamera camera = FreeCamera(FreeCameraCI_Default());
	FixedCamera camera = FixedCamera(FixedCameraCI_Default());
	// ImGui initialization
	ImGuiWrapper imguiWrapper = ImGuiWrapper();
	imguiWrapper.init();


	

	ImageLoader imageLoader = ImageLoader(gResourceBaseDir + "/textures");

	// Images
	FramebufferImage offscreenImage = FramebufferImage(&gState.heap,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent);
	FramebufferImage depthImage     = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT, gState.io.extent);
	FramebufferImage    lineColorImg          = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, gState.io.format, gState.io.extent);
	FramebufferImage linePosImg     = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, gState.io.extent);
	FramebufferImage pinIdImage     = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R32_UINT, gState.io.extent);
	VkSamplerCreateInfo samplerCI      = SamplerCreateInfo_Default(0.0);
	Sampler             defaultSampler        = Sampler(&gState.cache, samplerCI);

	//VkImageCreateInfo     envMapImageCreateInfo     = ImageCreateInfo_Default(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, cResolution2k, VK_FORMAT_R32G32B32A32_SFLOAT);
	//Image                 envMap                    = ImageVma(&gState.heap, envMapImageCreateInfo);
	//VkImageViewCreateInfo envMapImageViewCreateInfo = ImageViewCreateInfo_Default(envMap.img, VK_FORMAT_R32G32B32A32_SFLOAT);

	EnvironmentMap envMap = EnvironmentMap(&gState.heap,
	                                       ImageCreateInfo_Default(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, cResolution2k, VK_FORMAT_R32G32B32A32_SFLOAT),
	                                       ImageViewCreateInfo_Default(envMap.img, VK_FORMAT_R32G32B32A32_SFLOAT),
	                                       SamplerCreateInfo_Default(0.0));
	Sampler envMapSampler = envMap.getSampler();
	std::string         envMapName      = std::string("envmap/2k/autumn_field_2k.hdr");



#ifdef POST_PROCESSING
	FramebufferImage lastFrameImage = FramebufferImage(&gState.heap,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent);
	FramebufferImage pingPongImages[2] =
	    {
	        FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent),
	        FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, gState.io.format, gState.io.extent)
	};
#endif

	// Buffers
	Buffer         viewBuf     = BufferVma(&gState.heap, sizeof(View), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	GaussianBuffer gaussianBuf = GaussianBuffer(GAUSSIAN_COUNT, GAUSSIAN_MARGIN);
	PinBuffer      pinBuf      = PinBuffer(PIN_COUNT);
	Buffer         pinDirectionsBuffer;
	GridBuffer     pinGridBuf = GridBuffer(PIN_GRID_SIZE, PINS_PER_GRID_CELL, &pinBuf, &gaussianBuf);
	Buffer         pinTransmittanceBuf;
	Buffer            pinVertexBuffer = BufferVma(&gState.heap, sizeof(PosVertex) * 2 * PIN_COUNT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Buffer            pinIndexBuffer  = BufferVma(&gState.heap, sizeof(Index) * 2 * PIN_COUNT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Buffer            pinUsedBuffer   = BufferVma(&gState.heap, sizeof(uint32_t) * PIN_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	Rect2D<float> secondaryViewport = {0.8, 0.0, 0.2, 1.0};

	// Render passes
	std::vector<RenderPass *> renderPasses(3);
	ConfigurableRenderPassCI renderPassCI{};
	{
		renderPassCI.pDepthImage        = &depthImage;
		renderPassCI.pColorAttachments  = {&lineColorImg, &linePosImg};
		renderPassCI.colorClear         = {false, true};
		renderPassCI.colorClearValues   = {{1.0, 1.0, 1.0, 0.0}, {0.0, 0.0, 0.0, 0.0}};
		renderPassCI.colorInitialLayout     = {VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		renderPassCI.colorTargetLayout      = {VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		renderPassCI.relRenderArea         = {0.0, 0.0, 1.0, 1.0};
		renderPasses[RENDER_PASS_WIREFRAME] = new ConfigurableRenderPass(renderPassCI);
	}
	renderPasses[RENDER_PASS_VOLUME] = new DefaultRenderPass(&depthImage, &offscreenImage);
	{
		renderPassCI.pDepthImage            = &depthImage;
		renderPassCI.pColorAttachments      = {&offscreenImage, &pinIdImage};
		renderPassCI.colorClear             = {true, true};
		renderPassCI.colorClearValues       = {{0.9, 0.9, 0.9, 0.0}, {0,0,0,0}};
		renderPassCI.colorInitialLayout     = {VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED};
		renderPassCI.colorTargetLayout      = {VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		renderPassCI.relRenderArea          = secondaryViewport;
		//renderPassCI.relRenderArea          = {0.0, 0.0, 1.0, 1.0};
		renderPasses[RENDER_PASS_MATERIAL] = new ConfigurableRenderPass(renderPassCI);
	}
	for (auto &rp : renderPasses)
	{
		if (rp != nullptr)
		{
			rp->init();
		}
	}

	// Geometry
	std::vector<PosVertex> objCubeVertices, objSphereVertices, objLowPolySphereVertices;
	std::vector<Index>     objCubeIndices, objSphereIndices, objLowPolySphereIndices;
	loadObj(modelPathPrefix + "cube/cube.obj", objCubeVertices, objCubeIndices);
	loadObj(modelPathPrefix + "lowpoly_sphere/lowpoly_sphere.obj", objLowPolySphereVertices, objLowPolySphereIndices);
	loadObj(modelPathPrefix + "sphere/sphere.obj", objSphereVertices, objSphereIndices);
	Geometry_T<PosVertex> cubeGeom = Geometry_T<PosVertex>(&gState.heap, objCubeVertices, objCubeIndices);
	Geometry_T<PosVertex> sphereGeom = Geometry_T<PosVertex>(&gState.heap, objSphereVertices, objSphereIndices);
	Geometry_T<PosVertex> lowpolySphereGeom = Geometry_T<PosVertex>(&gState.heap, objLowPolySphereVertices, objLowPolySphereIndices);
	Geometry_T<PosVertex> pinGeom  = Geometry_T<PosVertex>(&gState.heap, pinVertexBuffer, PIN_COUNT * 2, pinIndexBuffer, PIN_COUNT * 2);

	// Build scene
	std::vector<Model*>     models;
	std::vector<Transform *> transforms;
	std::vector<uint32_t>    instanceCounts;
	// Add gaussian cube

	GaussianFog_M             gaussianFogMat           = GaussianFog_M(renderPasses[RENDER_PASS_VOLUME], &viewBuf, &gaussianBuf, &defaultSampler, &lineColorImg, &linePosImg);
	DefaulModel<PosVertex> gaussianFogCube       = DefaulModel<PosVertex>(&cubeGeom, &gaussianFogMat, RENDER_PASS_VOLUME);
	Transform                 gaussianFogCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(-0.5, -0.5, -0.5)));
	models.push_back(&gaussianFogCube);
	transforms.push_back(&gaussianFogCubeTransform);
	instanceCounts.push_back(1);


	WireframeSphere_M      sphereMat             = WireframeSphere_M(renderPasses[RENDER_PASS_WIREFRAME], &viewBuf);
	DefaulModel<PosVertex> wireFrameSphere = DefaulModel<PosVertex>(&lowpolySphereGeom, &sphereMat, RENDER_PASS_WIREFRAME);
	Transform              sphereTransform       = Transform(glm::mat4(1.0));
	models.push_back(&wireFrameSphere);
	transforms.push_back(&sphereTransform);
	instanceCounts.push_back(2);


	Pins_M                 pinMat          = Pins_M(renderPasses[RENDER_PASS_WIREFRAME], &viewBuf, &pinUsedBuffer);
	DefaulModel<PosVertex> pinMatModel     = DefaulModel<PosVertex>(&pinGeom, &pinMat, RENDER_PASS_WIREFRAME);
	Transform              pinMatTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0)));
	models.push_back(&pinMatModel);
	transforms.push_back(&pinMatTransform);
	instanceCounts.push_back(1);

	Gaussian_M             gaussianMat             = Gaussian_M(renderPasses[RENDER_PASS_MATERIAL], &viewBuf, &gaussianBuf, &envMapSampler, &envMap);
	DefaulModel<PosVertex> gaussianSphere          = DefaulModel<PosVertex>(&sphereGeom, &gaussianMat, RENDER_PASS_MATERIAL);
	Transform              gaussianSphereTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, -4.0, 0.0)));
	models.push_back(&gaussianSphere);
	transforms.push_back(&gaussianSphereTransform);
	instanceCounts.push_back(1);

	
	GaussianNNGrid_M       gaussianNNGridMat             = GaussianNNGrid_M(renderPasses[RENDER_PASS_MATERIAL], &viewBuf, &pinTransmittanceBuf, &pinGridBuf, &gaussianBuf, &envMapSampler, &envMap);
	DefaulModel<PosVertex> gaussianNNGridSphere          = DefaulModel<PosVertex>(&sphereGeom, &gaussianNNGridMat, RENDER_PASS_MATERIAL);
	Transform              gaussianNNGridSphereTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.5, 0.0)));
	models.push_back(&gaussianNNGridSphere);
	transforms.push_back(&gaussianNNGridSphereTransform);
	instanceCounts.push_back(1);


	GaussianNN_M           gaussianNNMat             = GaussianNN_M(renderPasses[RENDER_PASS_MATERIAL],
	                                                                &viewBuf, &pinBuf, &pinTransmittanceBuf, &pinDirectionsBuffer, &pinUsedBuffer, &gaussianBuf, &envMapSampler, &envMap,{{"METRIC_ANGLE_DISTANCE", ""}});
	DefaulModel<PosVertex> gaussianNNSphere          = DefaulModel<PosVertex>(&sphereGeom, &gaussianNNMat, RENDER_PASS_MATERIAL);
	Transform              gaussianNNSphereTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.5, 0.0)));
	models.push_back(&gaussianNNSphere);
	transforms.push_back(&gaussianNNSphereTransform);
	instanceCounts.push_back(PIN_COUNT_SQRT);


	GaussianNN_M           gaussianNN2Mat             = GaussianNN_M(renderPasses[RENDER_PASS_MATERIAL],
	                                                                 &viewBuf, &pinBuf, &pinTransmittanceBuf, &pinDirectionsBuffer, &pinUsedBuffer, &gaussianBuf, &envMapSampler, &envMap, {{"METRIC_DISTANCE_DISTANCE", ""}});
	DefaulModel<PosVertex> gaussianNN2Sphere          = DefaulModel<PosVertex>(&sphereGeom, &gaussianNN2Mat, RENDER_PASS_MATERIAL);
	Transform              gaussianNN2SphereTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 4.0, 0.0)));
	models.push_back(&gaussianNN2Sphere);
	transforms.push_back(&gaussianNN2SphereTransform);
	instanceCounts.push_back(PIN_COUNT_SQRT);


	// Upload data
	UniversalCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// Imgui
	{
		imguiWrapper.uploadResources(cmdBuf);
	}

	// Upload buffers
	{
		cubeGeom.upload(cmdBuf);
		sphereGeom.upload(cmdBuf);
		lowpolySphereGeom.upload(cmdBuf);
		gaussianBuf.upload(cmdBuf);
		pinBuf.upload(cmdBuf);
		cmdBuf.fillBuffer(pinUsedBuffer, 0);
		pinDirectionsBuffer = pinBuf.buildDirectionBuffer(cmdBuf);
		pinBuf.writeLineBuffers(cmdBuf, pinVertexBuffer, pinIndexBuffer);
	}

	// Upload image
	{
		imageLoader.mapImage(envMapName, true);
		cmdBuf.uploadImage(imageLoader.getData(), imageLoader.getDataSize(), envMap, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cmdBuf.imageMemoryBarrier(envMap);
		imageLoader.unmapImage();
	}

	// Transition images
	{
#ifdef POST_PROCESSING
		cmdBuf.transitionLayout(pingPongImages[0], VK_IMAGE_LAYOUT_GENERAL);
		cmdBuf.transitionLayout(pingPongImages[1], VK_IMAGE_LAYOUT_GENERAL);
		cmdBuf.transitionLayout(lastFrameImage, VK_IMAGE_LAYOUT_GENERAL);
#endif
		cmdBuf.transitionLayout(offscreenImage, VK_IMAGE_LAYOUT_GENERAL);
		cmdBuf.transitionLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		cmdBuf.transitionLayout(lineColorImg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cmdBuf.transitionLayout(linePosImg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	// Build buffers
	{
		pinGridBuf.build(cmdBuf);
		pinTransmittanceBuf = pinBuf.buildTransmittanceBuffer(cmdBuf, &gaussianBuf);
	}

	commitCmdBuffers(&cmdBuf, 1, &gState.frame->stack, gState.device.universalQueues[0]);
	vkDeviceWaitIdle(gState.device.logical);
	imguiWrapper.destroyStagingResources();

	// Main loop
	uint32_t  cnt = 0;
	View      view{};
	glm::mat4 lastViewMatrix = glm::mat4(1.0);
	//camera.mouseControl(0, 0);
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
		{
			camera.keyControl(0.016);
			if (gState.io.mouse.rightPressed)
			{
				camera.mouseControl(0.016);
			}
			view.update(cnt, camera, secondaryViewport, &gaussianFogCubeTransform);
		}
		sphereTransform = Transform( glm::translate( glm::mat4(1.0), camera.getFixpoint() ) * glm::scale( glm::mat4(1.0), glm::vec3(0.1) ));
		//pinMatTransform = Transform( glm::translate( glm::mat4(1.0), camera.getFixpoint() ));
		


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
					drawSurfInst.push_back(DrawSurfaceInstance(drawSurf[j], transforms[i], sizeof(Transform)));
				}
			}
			drawCalls.push_back(DrawCall(drawSurfInst));
		}
		UniversalCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		// Upload data
		cmdBuf.uploadData(&view, sizeof(View), viewBuf);
		cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		lineColorImg.update(&gState.heap, &gState.frame->stack, gState.io.extent);
		cmdBuf.transitionLayout(lineColorImg, VK_IMAGE_LAYOUT_GENERAL);
		envMap.render(cmdBuf, lineColorImg, viewBuf);
		// Render
		{
			for (size_t i = 0; i < drawCalls.size(); i++)
			{
				drawCalls[i].buildInstanceBuffer(cmdBuf, &gState.frame->stack);
			}
			std::sort(drawCalls.begin(), drawCalls.end());
			cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDEX_READ_BIT);


			depthImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
			offscreenImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
			linePosImg.update(&gState.heap, &gState.frame->stack, gState.io.extent);
			pinIdImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
			cmdBuf.transitionLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			//cmdBuf.transitionLayout(lineColorImg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			cmdBuf.transitionLayout(linePosImg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			cmdBuf.transitionLayout(offscreenImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			
			if (!drawCalls.empty())
			{
				int renderPassID = drawCalls[0].drawSurf.sortKey;
				renderPasses[renderPassID]->beginRender(cmdBuf);
				for (size_t i = 0; i < drawCalls.size(); i++)
				{
					if (drawCalls[i].drawSurf.sortKey != renderPassID)
					{
						renderPasses[renderPassID]->endRender(cmdBuf);
						if (renderPassID == RENDER_PASS_WIREFRAME)
						{
							cmdBuf.copyImage(lineColorImg, offscreenImage);
							cmdBuf.fillBuffer(pinUsedBuffer, 0);
						}
						renderPassID = drawCalls[i].drawSurf.sortKey;
						renderPasses[renderPassID]->beginRender(cmdBuf);
					}

					drawCalls[i].drawSurf.pMaterial->bind(cmdBuf);
					drawCalls[i].submit(cmdBuf);
				}
				renderPasses[renderPassID]->endRender(cmdBuf);
			}
		}


		//Update used pins

		{
			glm::uvec3           workGroupSize  = {16, 16, 1};
			glm::uvec3           resolution     = {gState.io.extent.width, gState.io.extent.height, 1};
			glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
			ComputePipelineState computeState{};
			computeState.specialisationEntrySizes = glm3VectorSizes();
			computeState.specializationData       = getByteVector(workGroupSize);

			DescriptorSetLayoutDefinition layoutDefinition{};
			layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
			layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_SAMPLER);
			layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
			computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
			computeState.shaderDef.path     = shaderPathPrefix + "write_used_pins.comp";
			computeState.shaderDef.args     = {{"PIN_COUNT", std::to_string(PIN_COUNT)}};
			ComputePipeline computePipeline = ComputePipeline(&gState.cache, computeState);
			cmdBuf.bindPipeline(computePipeline);
			cmdBuf.pushDescriptors(0, viewBuf, pinUsedBuffer, defaultSampler, (Image) pinIdImage);
			cmdBuf.dispatch(workGroupCount);
		}

#ifdef POST_PROCESSING
		// Post processing
		{
			pingPongImages[0].update(&gState.heap, &gState.frame->stack, gState.io.extent);
			pingPongImages[1].update(&gState.heap, &gState.frame->stack, gState.io.extent);
			lastFrameImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
			cmdBuf.transitionLayout(pingPongImages[0], VK_IMAGE_LAYOUT_GENERAL);
			cmdBuf.transitionLayout(pingPongImages[1], VK_IMAGE_LAYOUT_GENERAL);
			cmdBuf.transitionLayout(lastFrameImage, VK_IMAGE_LAYOUT_GENERAL);

			// cmdBuf.barrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
			if (gvar_use_gaus_blur.val.v_bool || gvar_use_exp_moving_average.val.v_bool)
			{
				cmdBuf.copyImage(offscreenImage, pingPongImages[0]);
				cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
				{
					// Post processing
					glm::uvec3           workGroupSize  = {16, 16, 1};
					glm::uvec3           resolution     = {gState.io.extent.width, gState.io.extent.height, 1};
					glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
					ComputePipelineState computeState{};
					computeState.specialisationEntrySizes = glm3VectorSizes();
					computeState.specializationData       = getByteVector(workGroupSize);

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
							computeState.shaderDef.name     = "gaussfilter1D.comp";
							computeState.shaderDef.args     = {{"FILTER_X", ""}};
							ComputePipeline computePipeline = ComputePipeline(&gState.cache, computeState);
							cmdBuf.bindPipeline(computePipeline);
							cmdBuf.pushDescriptors(0, viewBuf, pingPongImages[0], pingPongImages[1]);
							cmdBuf.dispatch(workGroupCount);
						}
						cmdBuf.barrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
						{
							// Gauss Y
							computeState.shaderDef.name     = "gaussfilter1D.comp";
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
						computeState.shaderDef.name     = "expMovingAverage.comp";
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
		}
#endif        // POST_PROCESSING

		cmdBuf.copyToSwapchain(offscreenImage);
		//cmdBuf.copyToSwapchain(lineColorImg);
		imguiWrapper.renderGui(cmdBuf);
		gState.swapBuffers(&cmdBuf);
	}
	// Cleanup
	vkDeviceWaitIdle(gState.device.logical);
	for (size_t i = 0; i < renderPasses.size(); i++)
	{
		if (renderPasses[i] != nullptr)
		{
			renderPasses[i]->destroy();
			delete renderPasses[i];
		}
	}
	imguiWrapper.destroy();
	gState.destroy();
	delete window;
}