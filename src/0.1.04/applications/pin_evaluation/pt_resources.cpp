#include "pt_resources.h"
#include <random>


extern GVar gvar_medium_density_scale;

const std::string env_map_path = "/envmap/2k/cloudy_dusky_sky_dome_2k.hdr";
const glm::uvec2  env_map_pdf_resolution = glm::uvec2(64, 64);

const std::string model_path   = "under_the_c/scene_1.obj";
const glm::mat4   model_matrix           = getMatrix(vec3(0, 0.2, -0.3), vec3(0.0, 180.0, 0.0), 0.1);

const std::string medium_path           = "csafe_heptane_302x302x302_uint8.raw";
const glm::uvec3  medium_dimensions     = glm::uvec3(302, 302, 302); // currently must be equal
const uint32_t    medium_instance_count = 50;

std::vector<GLSLInstance> generateInstanceDistributionV1(uint32_t count, uint32_t seed)
{
	std::vector<GLSLInstance>             mediumInstances(count);
	std::mt19937                          rngGen(seed);
	std::uniform_real_distribution<float> dist(0.0, 1.0);
	for (uint32_t i = 0; i < count; i++)
	{
		vec3 randomPos            = vec3(3.0) * (vec3(dist(rngGen), dist(rngGen), dist(rngGen)) - vec3(0.5));
		randomPos.y               = 0.2;
		float randomScale         = dist(rngGen) * 0.8 + 0.1;
		mediumInstances[i].mat    = getMatrix(randomPos, vec3(0, 0, -90), randomScale);
		mediumInstances[i].invMat = glm::inverse(mediumInstances[i].mat);
		mediumInstances[i].color  = vec3(1.0, 1.0, 1.0);
		mediumInstances[i].cullMask = 0xFF;
	}
	return mediumInstances;
}

TraceResources cmdLoadResources(CmdBuffer cmdBuf, IResourcePool* pPool)
{
	TraceResources resources{};
	USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance>();

	// Geometry
	sceneBuilder.loadEnvMap(env_map_path, env_map_pdf_resolution);
	GLSLInstance instance{};
	instance.cullMask = 0xFF;
	instance.mat      = model_matrix;
	instance.invMat   = glm::inverse(model_matrix);
	sceneBuilder.addModel(cmdBuf, model_path, &instance, 1);
	resources.sceneData = sceneBuilder.create(cmdBuf, pPool);
	resources.sceneData.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, pPool));

	// Medium
	VkExtent3D mediumExtent{medium_dimensions.x, medium_dimensions.y, medium_dimensions.z};
	resources.mediumTexture = createImage(pPool, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, mediumExtent);
	cmdTransitionLayout(cmdBuf, resources.mediumTexture, VK_IMAGE_LAYOUT_GENERAL);
	Buffer scalarBuf;
	gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, scalarFieldPath + "csafe_heptane_302x302x302_uint8.raw", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	getCmdLoadScalarField(scalarBuf, resources.mediumTexture, gvar_medium_density_scale.val.v_float).exec(cmdBuf);

	// Medium Instances
	resources.mediumInstanceBuffer            = createBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	std::vector<GLSLInstance> mediumInstances = generateInstanceDistributionV1(medium_instance_count, 42);
	cmdWriteCopy(cmdBuf, resources.mediumInstanceBuffer, mediumInstances.data(), mediumInstances.size() * sizeof(GLSLInstance));
	BLAS boxBlas;
	if (!gState.uniqueResourceCache->fetch(boxBlas, "cmdBuildBoxBlas"))
	{
		boxBlas = cmdBuildBoxBlas(cmdBuf, pPool);
		gState.uniqueResourceCache->add(boxBlas, "cmdBuildBoxBlas");
	}
	resources.mediumTlas = createTopLevelAS(pPool, mediumInstances.size());
	default_scene::cmdBuildBoxIntersector<GLSLInstance>(cmdBuf, boxBlas, resources.mediumInstanceBuffer, mediumInstances.size(), resources.mediumTlas);
	return resources;
}