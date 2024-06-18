#pragma once
#include <vka/vka.h>
#include <random>
#include "shaderStructs.h"
#include "config.h"
#define PI 3.14159265359
using namespace vka;
GVar gvar_use_pins{"show pins", 0, GVAR_ENUM, GVAR_APPLICATION, {"None", "All", "Grid", "Nearest Neighbor 1", "Nearest Neighbor 2"}};
GVar gvar_pin_selection_coef{"pin selection coef", 1.0f, GVAR_UNORM, GVAR_APPLICATION};
GVar gvar_ray_lenght{"secondary ray length", 1.0f, GVAR_UNORM, GVAR_APPLICATION};
GVar gvar_positional_jitter{"positional_jitter", 0.0f, GVAR_UNORM, GVAR_APPLICATION};
GVar gvar_angular_jitter{"angular", 0.0f, GVAR_UNORM, GVAR_APPLICATION};

GVar gvar_use_env_map{"use env map", 1, GVAR_BOOL, GVAR_APPLICATION};
GVar gvar_reload{"reload", 0, GVAR_EVENT, GVAR_APPLICATION};

// POST PROCESSING
GVar gvar_use_exp_moving_average{"use exponential moving average", false, GVAR_BOOL, GVAR_APPLICATION};
GVar gvar_use_gaus_blur{"use gauss blur", false, GVAR_BOOL, GVAR_APPLICATION};
GVar gvar_exp_moving_average_coef{"exp moving average coef", 0.0f, GVAR_UNORM, GVAR_APPLICATION};


struct AppConfig
{
	uint32_t gaussianCount = 30;
	uint32_t gaussianMargin = 0.2f;
	uint32_t pinsPerGridCell = 20;
	uint32_t pinsGridSize = 10;
	uint32_t pinCountSqrt = 50;
	uint32_t inline pinCount() { return pinCountSqrt * pinCountSqrt; }
	uint32_t gaussFilterRadius = 4;


	Transform gaussianFogCubeTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(-0.5, -0.5, -0.5)));
	//Transform sphereTransform          = Transform(glm::mat4(1.0));
	Transform pinMatTransform          = Transform(glm::mat4(1.0));
	Transform gaussianSphereTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, -4.0, 0.0)));
	Transform gaussianNNGridSphereTransform = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.5, 0.0)));
	Transform gaussianNNSphereTransform     = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.5, 0.0)));
	Transform gaussianNN2SphereTransform    = Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 4.0, 0.0)));
	Rect2D<float> relSecondaryViewport = {0.8, 0.0, 0.2, 1.0};
};

struct AppData
{
	uint32_t    cnt;
	FixedCamera camera = FixedCameraCI_Default();
	SamplerDefinition defaultSampler;
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


	void init(vka::IResourcePool* pPool)
	{
		cnt              = 0;
		camera           = FixedCamera(FixedCameraCI_Default());
		defaultSampler = SamplerDefinition();
		// StorageBuffers
		viewBuf             = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		gaussianBuf         = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinBuf              = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinDirectionsBuffer = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinGridBuf          = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinTransmittanceBuf = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pinUsedBuffer       = vkaCreateBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);


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
		Transform sphereTransform = Transform(glm::translate(glm::mat4(1.0), camera.getFixpoint()) * glm::scale(glm::mat4(1.0), glm::vec3(0.1)));
		// Update view
		{
			View *view                   = (View *) vkaMapStageing(viewBuf, sizeof(View));
			view->width                  = gState.io.extent.width;
			view->height                 = gState.io.extent.height;
			view->frameCounter           = cnt;
			view->camPos                 = glm::vec4(camera.getPosition(), 1.0);
			view->viewMat                = camera.getViewMatrix();
			view->inverseViewMat         = glm::inverse(view->viewMat);
			view->projectionMat          = glm::perspective(glm::radians(60.0f), (float) gState.io.extent.width / (float) gState.io.extent.height, 0.1f, 500.0f);
			view->inverseProjectionMat   = glm::inverse(view->projectionMat);
			view->cube                   = Cube{glm::mat4(1.0), glm::mat4(1.0)};
			view->showPins               = gvar_use_pins.val.v_int;
			view->pinSelectionCoef       = gvar_pin_selection_coef.val.v_float;
			view->expMovingAverageCoef   = gvar_exp_moving_average_coef.val.v_float;
			view->secondaryWidth         = config.relSecondaryViewport.width * gState.io.extent.width;
			view->secondaryHeight        = config.relSecondaryViewport.height * gState.io.extent.height;
			view->secondaryProjectionMat = glm::perspective(glm::radians(60.0f), (float) view->secondaryWidth / (float) view->secondaryHeight, 0.1f, 500.0f);
			view->probe                  = glm::vec4(camera.getFixpoint(), 1.0);
			view->fogModelMatrix         = config.gaussianFogCubeTransform.mat;
			view->fogInvModelMatrix      = config.gaussianFogCubeTransform.invMat;
			view->secRayLength           = gvar_ray_lenght.val.v_float;
			view->positionalJitter       = gvar_positional_jitter.val.v_float;
			view->angularJitter          = gvar_angular_jitter.val.v_float;
			view->useEnvMap              = gvar_use_env_map.val.v_bool;
			vkaUnmap(viewBuf);
			vkaCmdUpload(cmdBuf, viewBuf);
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
		if (gvar_reload.val.v_bool || cnt == 0)
		{
			// Recompute gaussians
			uint32_t pinCount = config.pinCountSqrt * config.pinCountSqrt;
			std::mt19937                          gen32(42);
			std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);
			{
				Gaussian                             *gaussiansData = static_cast<Gaussian *>(vkaMapStageing(gaussianBuf, sizeof(Gaussian) * config.gaussianCount));
				for (size_t i = 0; i < config.gaussianCount; i++)
				{
					gaussiansData[i].mean.x   = config.gaussianMargin + (1.0 - 2.0 * config.gaussianMargin) * unormDistribution(gen32);
					gaussiansData[i].mean.y   = config.gaussianMargin + (1.0 - 2.0 * config.gaussianMargin) * unormDistribution(gen32);
					gaussiansData[i].mean.z   = config.gaussianMargin + (1.0 - 2.0 * config.gaussianMargin) * unormDistribution(gen32);
					float standardDeviation   = 0.1 + 0.4 * unormDistribution(gen32);
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
					// glm::vec3 origin = glm::vec3(x.x, y.x, z.x) + glm::vec3(0.5);
					glm::vec3 origin = glm::vec3(x.x, y.x, z.x);
					origin *= 0.866025403784;        // sqrt(3)/2
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
		vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		cnt++;
	 }
};