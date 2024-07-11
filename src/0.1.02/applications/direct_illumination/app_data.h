#pragma once
#include <vka/vka.h>
#include <random>
#include "shaderStructs.h"
#include "config.h"
#include <filesystem>
#define PI 3.14159265359
using namespace vka;
GVar gvar_pin_selection_coef{"pin selection coef", 0.5f, GVAR_UNORM, GVAR_RUNTIME_SETTINGS};
GVar gvar_ray_lenght{"secondary ray length", 1.0f, GVAR_UNORM, GVAR_RUNTIME_SETTINGS};
GVar gvar_positional_jitter{"positional_jitter", 0.0f, GVAR_UNORM, GVAR_RUNTIME_SETTINGS};
GVar gvar_angular_jitter{"angular", 0.0f, GVAR_UNORM, GVAR_RUNTIME_SETTINGS};
GVar gvar_gaussian_weight{"gaussian weight", 0.2f, GVAR_UNORM, GVAR_LOAD_SETTINGS};



GVar gvar_gaussian_count{"gaussian count", 30, GVAR_UINT, GVAR_LOAD_SETTINGS};
GVar gvar_gaussian_margin{"gaussian margin", 0.2f, GVAR_UNORM, GVAR_LOAD_SETTINGS};
GVar gvar_pins_per_grid_cell{"pins per grid cell", 20, GVAR_UINT, GVAR_LOAD_SETTINGS};
GVar gvar_pins_grid_size{"pins grid size", 10, GVAR_UINT, GVAR_LOAD_SETTINGS};
GVar gvar_pin_count_sqrt{"pin count sqrt", 50, GVAR_UINT, GVAR_LOAD_SETTINGS};
GVar gvar_pin_transmittance_steps{"transmittance_steps", 16, GVAR_UINT, GVAR_LOAD_SETTINGS};
GVar gvar_gaussian_std_deviation{"std deviation", 0.5f, GVAR_UNORM, GVAR_LOAD_SETTINGS};
GVar gvar_reload{"reload", 0, GVAR_EVENT, GVAR_LOAD_SETTINGS};

GVar gvar_use_pins{"show pins", 0, GVAR_ENUM, GVAR_WINDOW_SETTINGS, {"None", "All", "Grid", "Nearest Neighbor 1", "Nearest Neighbor 2"}};
GVar gvar_env_map{"Envmap", 0, GVAR_ENUM, GVAR_WINDOW_SETTINGS, {"None"}};
GVar gvar_show_cursor{"use gaussian", 1, GVAR_ENUM, GVAR_WINDOW_SETTINGS, {"None", "Inner Sphere", "Outher Sphere"}};
GVar gvar_show_gaussian{"gaussian", 1, GVAR_BOOL, GVAR_WINDOW_SETTINGS};
GVar gvar_show_grid{"grid", 1, GVAR_BOOL, GVAR_WINDOW_SETTINGS};
GVar gvar_show_nn1{"nearest neighbor (distance/angle)", 1, GVAR_BOOL, GVAR_WINDOW_SETTINGS};
GVar gvar_show_nn2{"nearest neighbor (distance/distance)", 1, GVAR_BOOL, GVAR_WINDOW_SETTINGS};

GVar gvar_render_mode{"render mode", 2, GVAR_ENUM, GVAR_WINDOW_SETTINGS, {"Angle Transmittance", "Reflection Transmittance", "Volume Transmittance"}};
GVar gvar_transmittance_mode{"transmittance mode", 0, GVAR_ENUM, GVAR_WINDOW_SETTINGS, {"Exact", "Grid", "Nearest Neighbor 1", "Nearest Neighbor 2"}};
GVar gvar_volume_type{"volume type", 1, GVAR_ENUM, GVAR_WINDOW_SETTINGS, {"Gaussian", "Raymarched"}};
GVar gvar_accumulate{"accumulation mode", 0, GVAR_BOOL, GVAR_WINDOW_SETTINGS};


GVar gvar_perlin_scale0{"scale 0", 0.5f, GVAR_UNORM, GVAR_PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_scale1{"scale 1", 0.5f, GVAR_UNORM, GVAR_PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_frequency0{"frequency 0", 0.5f, GVAR_UNORM, GVAR_PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_frequency1{"frequency 1", 0.5f, GVAR_UNORM, GVAR_PERLIN_NOISE_SETTINGS};
GVar gvar_perlin_falloff{"Falloff", false, GVAR_BOOL, GVAR_PERLIN_NOISE_SETTINGS};



// POST PROCESSING
//GVar gvar_use_exp_moving_average{"use exponential moving average", false, GVAR_BOOL, GVAR_APPLICATION};
//GVar gvar_use_gaus_blur{"use gauss blur", false, GVAR_BOOL, GVAR_APPLICATION};
//GVar gvar_exp_moving_average_coef{"exp moving average coef", 0.0f, GVAR_UNORM, GVAR_APPLICATION};


struct PerlinNoiseConfig
{
	float scale[2];
	float frequency[2];
	bool  falloff;

	bool operator==(const PerlinNoiseConfig &other) const
	{
		return falloff == other.falloff && scale[0] == other.scale[0] && scale[1] == other.scale[1] && frequency[0] == other.frequency[0] && frequency[1] == other.frequency[1];
	}

	bool update()
	{
		PerlinNoiseConfig newConf = PerlinNoiseConfig{gvar_perlin_scale0.val.v_float, gvar_perlin_scale1.val.v_float, gvar_perlin_frequency0.val.v_float, gvar_perlin_frequency1.val.v_float, gvar_perlin_falloff.val.v_bool};
		if (newConf == *this)
		{
			return false;
		}
		*this = newConf;
		return true;
	}
};


struct AppConfig
{
	uint32_t gaussianCount = 30;
	float gaussianMargin = 0.2f;
	uint32_t pinsPerGridCell = 20;
	uint32_t pinsGridSize = 10;
	uint32_t pinCountSqrt = 50;
	uint32_t pinTransmittanceSteps = 64;
	uint32_t inline pinCount() { return pinCountSqrt * pinCountSqrt; }
	uint32_t gaussFilterRadius = 4;


	//Transform gaussianFogCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(-0.5, -0.5, -0.5)));
	Transform gaussianFogCubeTransform = Transform(glm::mat4(1.0));
	Transform pinMatTransform          = Transform(glm::mat4(1.0));

	Transform gaussianSphereTransform;
	Transform gaussianNNGridSphereTransform;
	Transform gaussianNNSphereTransform;
	Transform gaussianNN2SphereTransform;

	VkRect2D_OP mainViewport;
	VkRect2D_OP guiViewport;
	VkRect2D_OP materialViewPort;

	glm::mat4 mainProjectionMat;
	glm::mat4 matrialProjectionMat;


	void update()
	{
		glm::vec3                pos   = glm::vec3(-4.0, 0.0, 0.0);
		glm::vec3                step  = glm::vec3(2.5, 0.0, 0.0);
		std::vector<Transform *> items = {&gaussianSphereTransform, &gaussianNNGridSphereTransform, &gaussianNNSphereTransform, &gaussianNN2SphereTransform};
		for (int i = 0; i < 4; i++)
		{
			*items[i] = Transform(glm::translate(glm::mat4(1.0), pos));
			pos += step;
		}


		if (gvar_reload.val.v_bool)
		{
			gaussianCount   = gvar_gaussian_count.val.v_uint;
			gaussianMargin  = gvar_gaussian_margin.val.v_float;
			pinsPerGridCell = gvar_pins_per_grid_cell.val.v_uint;
			pinsGridSize    = gvar_pins_grid_size.val.v_uint;
			pinCountSqrt    = gvar_pin_count_sqrt.val.v_uint;
			pinTransmittanceSteps = gvar_pin_transmittance_steps.val.v_uint;
		}

		if (gvar_render_mode.val.v_int == 0)
		{
			mainViewport = vkaGetScissorRect(0.2, 0.0, 1.0, 0.8);
		}
		else
		{
			mainViewport = vkaGetScissorRect(0.2, 0.0, 1.0, 1.0);
		}
		guiViewport  = vkaGetScissorRect(0.0, 0.0, 0.2, 1.0);
		materialViewPort = vkaGetScissorRect(0.205, 0.81, 0.79, 0.18);

		mainProjectionMat = glm::perspective(glm::radians(60.0f), (float) mainViewport.extent.width / (float) mainViewport.extent.height, 0.1f, 500.0f);
		matrialProjectionMat = glm::perspective(glm::radians(30.0f), (float) materialViewPort.extent.width / (float) materialViewPort.extent.height, 0.1f, 500.0f);

	}
};

struct AppData
{

	TextureCache* pTextureCache;
	FastDrawState* pFastDrawState; 


	uint32_t    cnt;
	FixedCamera camera = FixedCameraCI_Default();
	SamplerDefinition defaultSampler;
	SamplerDefinition volumeDataSampler;
	VkaBuffer viewBuf;
	VkaBuffer gaussianBuf;
	VkaBuffer pinBuf;
	VkaBuffer pinDirectionsBuffer;
	VkaBuffer pinGridBuf;
	VkaBuffer pinTransmittanceBuf;
	VkaBuffer pinVertexBuffer;
	VkaBuffer pinIndexBuffer;
	VkaBuffer pinUsedBuffer;

	VkaBuffer gaussianFogCubeTransformBuf;
	VkaBuffer sphereTransformBuf;
	VkaBuffer pinMatTransformBuf;
	VkaBuffer gaussianSphereTransformBuf;
	VkaBuffer gaussianNNGridSphereTransformBuf;
	VkaBuffer gaussianNNSphereTransformBuf;
	VkaBuffer gaussianNN2SphereTransformBuf;

	VkaImage envMap;
	VkaBuffer pdfHorizontal;
	VkaBuffer pdfVertical;
	uint32_t envMapIndexLastFrame = 0;

	VkaImage accumulationImage;

	// New
	CamConst mainCamConst;
	ViewConst mainViewConst;

	GuiVar guiVar;
	Transform cubeTransform;

	// main view

	Transform volumeTransform;

	VkaBuffer camConstBuf;
	VkaBuffer viewConstBuf;
	VkaBuffer guiVarBuf;
	VkaBuffer cubeTransformBuf;


	// Histogram
	bool startRegionSelect = false;
	bool regionSelected = false;
	bool histogramLoaded = false;

	bool startRegionSelectAccum = false;
	bool regionSelectedAccum    = false;
	bool accumulationLoaded            = false;
	uint32_t accumulationCount = 0;
	glm::uvec2 regionStartAccum;
	glm::uvec2 regionEndAccum;
	bool       resetAccumulation = true;

	VkaBuffer histogramBuffer;
	VkaBuffer histogramAverageBuffer;
	glm::uvec2 regionStart;
	glm::uvec2 regionEnd;


	// Ray marching

	VkaImage volumeImage;
	VkaBuffer pinTransmittanceBufV2;

	PerlinNoiseConfig perlinNoiseConfig;

	void init(vka::IResourcePool* pPool, vka::TextureCache* textureCache, FastDrawState* fastDrawState)
	{
		cnt              = 0;
		camera           = FixedCamera(FixedCameraCI_Default());
		defaultSampler = SamplerDefinition();
		volumeDataSampler = SamplerDefinition();
		volumeDataSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		volumeDataSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		volumeDataSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		volumeDataSampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		//defaultSampler.borderColor
		// StorageBuffers
		viewBuf             = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		gaussianBuf         = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinBuf              = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinDirectionsBuffer = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinGridBuf          = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinTransmittanceBuf = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinUsedBuffer       = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		pinTransmittanceBufV2 = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		// VertexBuffers/IndexBuffers
		pinVertexBuffer = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		pinIndexBuffer  = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		// Instance/Buffers
		gaussianFogCubeTransformBuf      = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		sphereTransformBuf               = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		pinMatTransformBuf               = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		gaussianSphereTransformBuf       = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		gaussianNNGridSphereTransformBuf = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		gaussianNNSphereTransformBuf     = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		gaussianNN2SphereTransformBuf    = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);


		camConstBuf = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		viewConstBuf = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		guiVarBuf    = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		cubeTransformBuf = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		histogramBuffer = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		histogramAverageBuffer = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		accumulationImage = vkaCreateImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VkExtent2D{10, 10});

		volumeImage = vkaCreateImage(pPool, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VkExtent3D{256, 256, 256});

		pTextureCache = textureCache;
		pdfHorizontal = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		pdfVertical   = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		pFastDrawState = fastDrawState;

	}

	void update(VkaCommandBuffer cmdBuf, AppConfig config)
	{
		Mappable pinsLocal = nullptr;
		// Update camera
		{
			camera.keyControl(0.016);
			if (gState.io.mouse.rightPressed)
			{
				camera.mouseControl(0.016);
			}
		}
		if ((!startRegionSelect) && gState.io.mouse.leftEvent && gState.io.mouse.leftPressed && gState.io.keyPressed[GLFW_KEY_E])
		{
			startRegionSelect = true;
			regionSelected = false;
			regionStart = gState.io.mouse.pos;
		}
		if (startRegionSelect && gState.io.mouse.leftEvent && !gState.io.mouse.leftPressed)
		{
			startRegionSelect = false;
			regionSelected = true;
			histogramLoaded = false;
			regionEnd = gState.io.mouse.pos;
		}

		if ((!startRegionSelectAccum) && gState.io.mouse.leftEvent && gState.io.mouse.leftPressed && gState.io.keyPressed[GLFW_KEY_F])
		{
			accumulationCount      = 0;
			startRegionSelectAccum = true;
			regionSelectedAccum    = false;
			regionStartAccum       = gState.io.mouse.pos;
		}
		if (startRegionSelectAccum && gState.io.mouse.leftEvent && !gState.io.mouse.leftPressed)
		{
			startRegionSelectAccum = false;
			regionSelectedAccum    = true;
			accumulationLoaded   = false;
			regionEndAccum         = gState.io.mouse.pos;
		}





		Transform sphereTransform = Transform(glm::translate(glm::mat4(1.0), camera.getFixpoint()) * glm::scale(glm::mat4(1.0), glm::vec3(0.1)));
		// Update view
		{
			View *view                   = (View *) vkaMapStageing(viewBuf, sizeof(View));
			view->width                  = gState.io.extent.width;        // config.mainViewport.extent.width;
			view->height                 = gState.io.extent.height;        // config.mainViewport.extent.height;
			view->frameCounter           = cnt;
			view->camPos                 = glm::vec4(camera.getPosition(), 1.0);
			view->viewMat                = camera.getViewMatrix();
			view->inverseViewMat         = glm::inverse(view->viewMat);
			view->projectionMat          = config.mainProjectionMat;
			view->inverseProjectionMat   = glm::inverse(view->projectionMat);
			view->cube                   = Cube{glm::mat4(1.0), glm::mat4(1.0)};
			view->showPins               = gvar_use_pins.val.v_int;
			view->pinSelectionCoef       = gvar_pin_selection_coef.val.v_float;
			view->expMovingAverageCoef   = 0.99;        // gvar_exp_moving_average_coef.val.v_float;
			view->secondaryWidth         = config.materialViewPort.extent.width;
			view->secondaryHeight        = config.materialViewPort.extent.height;
			view->secondaryProjectionMat = config.matrialProjectionMat;
			view->probe                  = glm::vec4(camera.getFixpoint(), 1.0);
			view->fogModelMatrix         = config.gaussianFogCubeTransform.mat;
			view->fogInvModelMatrix      = config.gaussianFogCubeTransform.invMat;
			view->secRayLength           = gvar_ray_lenght.val.v_float;
			view->positionalJitter       = gvar_positional_jitter.val.v_float;
			view->angularJitter          = gvar_angular_jitter.val.v_float;
			view->useEnvMap              = gvar_env_map.val.v_uint;
			vkaUnmap(viewBuf);
			vkaCmdUpload(cmdBuf, viewBuf);
		}
		// Update shaderConst (will replace view)
		{
			mainViewConst.width  = config.mainViewport.extent.width;
			mainViewConst.height = config.mainViewport.extent.height;
			mainViewConst.frameIdx = cnt;

			mainCamConst.viewMat                  = camera.getViewMatrix();
			mainCamConst.inverseViewMat            = glm::inverse(mainCamConst.viewMat);
			mainCamConst.projectionMat             = config.mainProjectionMat;
			mainCamConst.inverseProjectionMat      = glm::inverse(mainCamConst.projectionMat);
			mainCamConst.camPos                    = glm::vec4(camera.getPosition(), 1.0);
			mainCamConst.camFixpoint               = glm::vec4(camera.getFixpoint(), 1.0);

			guiVar.useEnvMap              = gvar_env_map.val.v_uint;
			guiVar.secRayLength           = gvar_ray_lenght.val.v_float;
			guiVar.positionalJitter       = gvar_positional_jitter.val.v_float;
			guiVar.angularJitter          = gvar_angular_jitter.val.v_float;
			guiVar.showPins               = gvar_use_pins.val.v_int;
			guiVar.pinSelectionCoef       = gvar_pin_selection_coef.val.v_float;
			guiVar.gaussianWeight       = gvar_gaussian_weight.val.v_float;
			guiVar.stdDeviation           = gvar_gaussian_std_deviation.val.v_float;

			/*guiVar.perlinScale[0] = gvar_perlin_scale0.val.v_float;
			guiVar.perlinScale[1] = gvar_perlin_scale1.val.v_float;
			guiVar.perlinFrequency[0] = gvar_perlin_frequency0.val.v_float;
			guiVar.perlinFrequency[1] = gvar_perlin_frequency1.val.v_float;*/

			guiVar.perlinScale0     = gvar_perlin_scale0.val.v_float;
			guiVar.perlinScale1     = gvar_perlin_scale1.val.v_float;
			guiVar.perlinFrequency0 = gvar_perlin_frequency0.val.v_float;
			guiVar.perlinFrequency1 = gvar_perlin_frequency1.val.v_float;
			guiVar.perlinFalloff	= gvar_perlin_falloff.val.v_bool;

			vkaWriteStaging(guiVarBuf, &guiVar, sizeof(GuiVar));
			vkaCmdUpload(cmdBuf, guiVarBuf);

			cubeTransform = config.gaussianFogCubeTransform;
		}
		// Update instance buffers
		{
			vkaWriteStaging(gaussianFogCubeTransformBuf, &config.gaussianFogCubeTransform, sizeof(Transform));
			vkaFillStaging(sphereTransformBuf, &sphereTransform, sizeof(Transform), 2);

			vkaWriteStaging(pinMatTransformBuf, &config.pinMatTransform, sizeof(Transform));
			vkaWriteStaging(gaussianSphereTransformBuf, &config.gaussianSphereTransform, sizeof(Transform));
			vkaWriteStaging(gaussianNNGridSphereTransformBuf, &config.gaussianNNGridSphereTransform, sizeof(Transform));

			vkaFillStaging(gaussianNNSphereTransformBuf, &config.gaussianNNSphereTransform, sizeof(Transform), config.pinCountSqrt);
			vkaFillStaging(gaussianNN2SphereTransformBuf, &config.gaussianNN2SphereTransform, sizeof(Transform), config.pinCountSqrt);

			vkaCmdUpload(cmdBuf, gaussianFogCubeTransformBuf);
			vkaCmdUpload(cmdBuf, sphereTransformBuf);
			vkaCmdUpload(cmdBuf, pinMatTransformBuf);
			vkaCmdUpload(cmdBuf, gaussianSphereTransformBuf);
			vkaCmdUpload(cmdBuf, gaussianNNGridSphereTransformBuf);
			vkaCmdUpload(cmdBuf, gaussianNNSphereTransformBuf);
			vkaCmdUpload(cmdBuf, gaussianNN2SphereTransformBuf);
		}
		vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		uint32_t pinCount = config.pinCountSqrt * config.pinCountSqrt;
		if (gvar_reload.val.v_bool || cnt == 0)
		{
			// Recompute gaussians
			
			std::mt19937                          gen32(42);
			std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);
			{
				Gaussian                             *gaussiansData = static_cast<Gaussian *>(vkaMapStageing(gaussianBuf, sizeof(Gaussian) * config.gaussianCount));
				for (size_t i = 0; i < config.gaussianCount; i++)
				{
					gaussiansData[i].mean.x   = config.gaussianMargin + (1.0 - 2.0 * config.gaussianMargin) * unormDistribution(gen32);
					gaussiansData[i].mean.y   = config.gaussianMargin + (1.0 - 2.0 * config.gaussianMargin) * unormDistribution(gen32);
					gaussiansData[i].mean.z   = config.gaussianMargin + (1.0 - 2.0 * config.gaussianMargin) * unormDistribution(gen32);
					float standardDeviation   = gvar_gaussian_std_deviation.val.v_float*0.2;        // 0.1 + 0.4 * unormDistribution(gen32);
					gaussiansData[i].variance = standardDeviation * standardDeviation;
				}
				vkaUnmap(gaussianBuf);
				vkaCmdUpload(cmdBuf, gaussianBuf);
			}
			// Recompute pins
			{
				Pin                                  *pinData  = static_cast<Pin *>(vkaMapStageing(pinBuf, sizeof(Pin) * pinCount));
				for (size_t i = 0; i < pinCount; i++)
				{
					pinData[i].phi.x   = 2.0 * PI * unormDistribution(gen32);
					pinData[i].phi.y   = 2.0 * PI * unormDistribution(gen32);
					pinData[i].theta.x = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
					pinData[i].theta.y = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
				}
				vkaUnmap(pinBuf);
				vkaCmdUpload(cmdBuf, pinBuf, pinsLocal);
			}

			vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

			// Pin transmittance
			{
				pinTransmittanceBuf->changeSize(pinCount);
				pinTransmittanceBuf->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
				pinTransmittanceBuf->recreate();
				ComputeCmd computeCmd;
				setDefaults(computeCmd, pinCount, shaderPath + "pins_eval_transmittance.comp",
				{
				           {"GAUSSIAN_COUNT", std::to_string(config.gaussianCount)},
				           {"PIN_COUNT", std::to_string(pinCount)}
				});
				addDescriptor(computeCmd, gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				addDescriptor(computeCmd, pinBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				addDescriptor(computeCmd, pinTransmittanceBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				vkaCmdCompute(cmdBuf, computeCmd);
			}

			// Pin directions
			{
				glm::vec4 *pinDirectionsData = static_cast<glm::vec4 *>(vkaMapStageing(pinDirectionsBuffer, sizeof(glm::vec4) * pinCount));
				Pin       *pinData           = static_cast<Pin *>(pinsLocal->map(0, pinBuf->getSize()));
				for (size_t i = 0; i < pinCount; i++)
				{
					glm::vec2 x      = sin(pinData[i].phi) * cos(pinData[i].theta);
					glm::vec2 y      = sin(pinData[i].phi) * sin(pinData[i].theta);
					glm::vec2 z      = cos(pinData[i].phi);
					glm::vec3 origin = glm::vec3(x.x, y.x, z.x) + glm::vec3(0.5);
					origin *= 0.866025403784;        // sqrt(3)/2
					glm::vec3 direction  = glm::normalize(glm::vec3(x.y - x.x, y.y - y.x, z.y - z.x));
					pinDirectionsData[i] = glm::vec4(direction, 0.0);
				}
				pinsLocal->unmap();
				vkaUnmap(pinDirectionsBuffer);
				vkaCmdUpload(cmdBuf, pinDirectionsBuffer);
			}
			// Line Buffers
			{
				PosVertex *vertexData = static_cast<PosVertex *>(vkaMapStageing(pinVertexBuffer, sizeof(PosVertex) * 2 * pinCount));
				Index     *indexData  = static_cast<Index *>(vkaMapStageing(pinIndexBuffer, sizeof(Index) * 2 * pinCount));
				Pin       *pinData    = static_cast<Pin *>(pinsLocal->map(0, pinBuf->getSize()));
				for (size_t i = 0; i < pinCount; i++)
				{
					glm::vec2 x = sin(pinData[i].phi) * cos(pinData[i].theta);
					glm::vec2 y = sin(pinData[i].phi) * sin(pinData[i].theta);
					glm::vec2 z = cos(pinData[i].phi);
					glm::vec3 origin = glm::vec3(x.x, y.x, z.x);
					origin *= 0.866025403784;        // sqrt(3)/2
					origin += glm::vec3(0.5);

					glm::vec3 direction   = glm::normalize(glm::vec3(x.y - x.x, y.y - y.x, z.y - z.x));
					vertexData[i * 2]     = PosVertex(origin + direction * 0.1f);
					vertexData[i * 2 + 1] = PosVertex(origin + direction * 0.866025403784f * 2.0f - direction * 0.1f);

					indexData[i * 2]      = i * 2;
					indexData[i * 2 + 1]  = i * 2 + 1;
				}
				pinsLocal->unmap();
				vkaUnmap(pinVertexBuffer);
				vkaUnmap(pinIndexBuffer);
				vkaCmdUpload(cmdBuf, pinVertexBuffer);
				vkaCmdUpload(cmdBuf, pinIndexBuffer);
			}
			vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			// Grid Buffer
			{
				pinGridBuf->changeSize(sizeof(PinGridEntry) * config.pinsPerGridCell * config.pinsGridSize * config.pinsGridSize * config.pinsGridSize);
				pinGridBuf->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
				pinGridBuf->recreate();
				ComputeCmd computeCmd;
				setDefaults(computeCmd, {config.pinsGridSize, config.pinsGridSize, config.pinsGridSize}, shaderPath + "pins_grid_gen.comp",
				            {{"PIN_GRID_SIZE", std::to_string(config.pinsGridSize)},
				             {"PIN_COUNT", std::to_string(pinCount)},
				             {"PINS_PER_GRID_CELL", std::to_string(config.pinsPerGridCell)}
					});
				addDescriptor(computeCmd, pinBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				addDescriptor(computeCmd, pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				vkaCmdCompute(cmdBuf, computeCmd);
			}
			// Pin used Buffer
			{
				pinUsedBuffer->changeSize(sizeof(uint32_t) * pinCount);
				pinUsedBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
				pinUsedBuffer->recreate();
			}
		}
		if (gvar_reload.val.v_bool || cnt == 0 || perlinNoiseConfig.update())
		{
			// Set up volume data
			{
				vkaCmdTransitionLayout(cmdBuf, volumeImage, VK_IMAGE_LAYOUT_GENERAL);

				ComputeCmd computeCmd;
				setDefaults(computeCmd, {256, 256, 256}, newShaderPath + "volume_gen.comp",
				            {{"VOLUME_SIZE", std::to_string(256)}});
				addDescriptor(computeCmd, volumeImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
				addDescriptor(computeCmd, guiVarBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				vkaCmdCompute(cmdBuf, computeCmd);

				vkaCmdTransitionLayout(cmdBuf, volumeImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			// Pin transmittance V2
			{
				pinTransmittanceBufV2->changeSize(pinCount * config.pinTransmittanceSteps * sizeof(float));
				pinTransmittanceBufV2->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
				pinTransmittanceBufV2->recreate();
				ComputeCmd computeCmd;
				setDefaults(computeCmd, pinCount, newShaderPath + "transmittance/ray_marched/pins_compute_transmittance.comp",
				            {{"PIN_TRANSMITTANCE_STEPS", std::to_string(config.pinTransmittanceSteps)},
				             {"PIN_COUNT", std::to_string(pinCount)}});
				addDescriptor(computeCmd, pinBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				addDescriptor(computeCmd, pinTransmittanceBufV2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				addDescriptor(computeCmd, volumeImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				addDescriptor(computeCmd, &volumeDataSampler, VK_DESCRIPTOR_TYPE_SAMPLER);
				vkaCmdCompute(cmdBuf, computeCmd);
			}
		}
		// Scan envmap
		if (gvar_env_map.val.v_uint != envMapIndexLastFrame || gvar_reload.val.v_bool || cnt == 0)
		{
			gvar_env_map.enumVal = {"None"};
			for (const auto &entry : std::filesystem::directory_iterator(texturePath + std::string("envmap/2k/")))
			{
				std::string name = entry.path().string();
				name             = std::string("envmap/2k/") + name.substr(name.find_last_of("/") + 1);
				gvar_env_map.enumVal.push_back(name);
			}
			gvar_env_map.val.v_uint = std::min(gvar_env_map.val.v_uint, uint32_t(gvar_env_map.enumVal.size() - 1));

			// Fetch envmap
			VKA_ASSERT(gvar_env_map.enumVal.size() >= 2);
			uint32_t envMapIdx = std::max(1u, gvar_env_map.val.v_uint);
			{
				envMap = pTextureCache->fetch(cmdBuf,
				                              gvar_env_map.enumVal[envMapIdx],
				                              VK_FORMAT_R32G32B32A32_SFLOAT,
				                              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				pFastDrawState->marginalize(cmdBuf, pdfHorizontal, pdfVertical, envMap, {64, 64});
			}

			envMapIndexLastFrame = gvar_env_map.val.v_uint;
		}
		vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		cnt++;

	 }
};