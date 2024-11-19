#include "pt_resources.h"
#include "ui.h"
#include <random>


extern GVar gvar_medium_density_scale;
extern GVar gvar_menu;

// UScene
GVar gvar_model_path{"Select model", modelPath + "under_the_c/scene_1.obj", GVAR_FILE_INPUT, GUI_CAT_SCENE_GENERAL, std::vector<std::string>({".obj", modelPath})};
GVar gvar_texture_path{"Select envmap", texturePath + "envmap/2k/cloudy_dusky_sky_dome_2k.hdr", GVAR_FILE_INPUT, GUI_CAT_SCENE_GENERAL, std::vector<std::string>({".hdr", texturePath})};

// Medium
GVar gvar_medium_path{"Select medium", texturePath + "csafe_heptane_302x302x302_uint8.raw", GVAR_FILE_INPUT, GUI_CAT_SCENE_MEDIUM, std::vector<std::string>({".raw", scalarFieldPath})};
GVar gvar_medium_shader_path{"Select medium shader", texturePath + "bla.comp", GVAR_FILE_INPUT, GUI_CAT_SCENE_MEDIUM, std::vector<std::string>({".comp", shaderPath})};
GVar gvar_medium_shader_params{"Med. s. params", std::string(""), GVAR_TEXT_INPUT, GUI_CAT_SCENE_MEDIUM};
GVar gvar_medium_load{"Load medium", false, GVAR_EVENT, GUI_CAT_SCENE_MEDIUM};

// Medium Instances
GVar gvar_medium_instance_shader_path{"Select medium instance shader", texturePath + "blub.comp", GVAR_FILE_INPUT, GUI_CAT_SCENE_MEDIUM_INSTANCES, std::vector<std::string>({".comp", shaderPath})};
GVar gvar_medium_instamce_shader_params{"MedInst. s. params", std::string(""), GVAR_TEXT_INPUT, GUI_CAT_SCENE_MEDIUM_INSTANCES};
GVar gvar_medium_inst_load{"Load medium inst", false, GVAR_EVENT, GUI_CAT_SCENE_MEDIUM_INSTANCES};

// Scene transforms
GVar gvar_model_pos{"Model Pos", {0, 0.2, -0.3}, GVAR_VEC3_RANGE, GUI_CAT_SCENE_TRANSFORMS, {-10.f, 10.f}};
GVar gvar_model_rot{"Model Rot", {0, 180, 0}, GVAR_VEC3_RANGE, GUI_CAT_SCENE_TRANSFORMS, {-180.f, 180.f}};
GVar gvar_model_scale{"Model Scale", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_TRANSFORMS, {0.01f, 1.f}};


glm::mat4 getModelMatrix()
{
	return getMatrix(gvar_model_pos.val.getVec3(), gvar_model_rot.val.getVec3(), gvar_model_scale.val.v_float);
}

ScalarFieldInfo getScalarFieldInfo(const std::string &path)
{
	VKA_ASSERT(path.find_last_of('/') != std::string::npos);
	std::string name = path.substr(path.find_last_of('/') + 1);
	name = name.substr(0, name.find_last_of('.'));

	ScalarFieldInfo info;
	if (name.find("uint8") != std::string::npos)
	{
		info.format = ScalarFieldFormat::UINT8;
	}
	else if (name.find("uint16") != std::string::npos)
	{
		info.format = ScalarFieldFormat::UINT16;
	}
	else
	{
		DEBUG_BREAK;
	}
	
	name = name.substr(0, name.find_last_of('_'));
	std::string dimStr = name.substr(name.find_last_of('_') + 1);

	VKA_ASSERT(
		dimStr.find('x') != std::string::npos
		&& dimStr.find_last_of('x') != std::string::npos && dimStr.find('x') != dimStr.find_last_of('x'));

	std::string dimStrX = dimStr.substr(0, dimStr.find('x'));
	std::string dimStrY = dimStr.substr(dimStr.find('x') + 1, dimStr.find_last_of('x') - dimStr.find('x') - 1);
	std::string dimStrZ = dimStr.substr(dimStr.find_last_of('x') + 1);
	info.extent         = {static_cast<uint32_t>(std::stoi(dimStr)), static_cast<uint32_t>(std::stoi(dimStrY)), static_cast<uint32_t>(std::stoi(dimStrZ))};
	return info;
}

const std::string default_env_map_path           = texturePath + "envmap/2k/cloudy_dusky_sky_dome_2k.hdr";
const glm::uvec2  env_map_pdf_resolution = glm::uvec2(64, 64);

const std::string default_model_path = modelPath + "under_the_c/scene_1.obj";
//const glm::mat4   model_matrix = getMatrix(vec3(0, 0.2, -0.3), vec3(0.0, 180.0, 0.0), 0.1);

const std::string default_medium_path   = scalarFieldPath + "csafe_heptane_302x302x302_uint8.raw";
const std::string default_medium_shader_path = shaderPath + "medium.comp.spv"; // todo
const glm::uvec3  medium_dimensions     = glm::uvec3(302, 302, 302);        // currently must be equal

const std::string default_medium_instance_shader_path = shaderPath + "medium_instance.comp.spv"; // todo
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

void TraceResources::cmdLoadSceneData(CmdBuffer cmdBuf, IResourcePool *pPool)
{
	if (resourcesTypes & TraceResourcesType_Scene)
	{
		sceneData.garbageCollect();
	}

	std::string model_path = default_model_path;
	if (std::filesystem::exists(gvar_model_path.val.v_char_array.data()))
	{
		model_path = gvar_model_path.val.v_char_array.data();
	}
	std::string env_map_path = default_env_map_path;
	if (std::filesystem::exists(gvar_texture_path.val.v_char_array.data()))
	{
		env_map_path = gvar_texture_path.val.v_char_array.data();
	}

	USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance>();
	// Geometry
	sceneBuilder.loadEnvMap(env_map_path, env_map_pdf_resolution); // todo
	GLSLInstance instance{};
	instance.cullMask = 0xFF;
	instance.mat      = getModelMatrix();
	instance.invMat   = glm::inverse(instance.mat);
	sceneBuilder.addModel(cmdBuf, model_path, &instance, 1);
	sceneData = sceneBuilder.create(cmdBuf, pPool);
	sceneData.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, pPool));

	resourcesTypes |= TraceResourcesType_Scene;
}

void TraceResources::cmdLoadMedium(CmdBuffer cmdBuf, IResourcePool *pPool)
{
	if (resourcesTypes & TraceResourcesType_Medium)
	{
		mediumTexture->garbageCollect();
	}

	std::string medium_path = default_medium_path;
	if (std::filesystem::exists(gvar_medium_path.val.v_char_array.data()))
	{
		medium_path = gvar_medium_path.val.v_char_array.data();
	}

	std::string medium_shader_path = default_medium_shader_path;
	if (std::filesystem::exists(gvar_medium_shader_path.val.v_char_array.data()))
	{
		medium_shader_path = gvar_medium_shader_path.val.v_char_array.data();
	}

	// Medium
	Buffer scalarBuf;
	gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, medium_path, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	mediumTexture = createImage3D(pPool, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	cmdLoadScalarField(cmdBuf, scalarBuf, mediumTexture, getScalarFieldInfo(medium_path));
	resourcesTypes |= TraceResourcesType_Medium;
}

void TraceResources::cmdLoadMediumInstances(CmdBuffer cmdBuf, IResourcePool *pPool)
{
	if (resourcesTypes & TraceResourcesType_MediumInstance)
	{
		mediumInstanceBuffer->garbageCollect();
		mediumTlas->garbageCollect();
	}

	std::string medium_instance_shader_path = default_medium_instance_shader_path;
	if (std::filesystem::exists(gvar_medium_instance_shader_path.val.v_char_array.data()))
	{
		medium_instance_shader_path = gvar_medium_instance_shader_path.val.v_char_array.data();
	}

	// Medium Instances
	mediumInstanceBuffer            = createBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	std::vector<GLSLInstance> mediumInstances = generateInstanceDistributionV1(medium_instance_count, 42);
	cmdWriteCopy(cmdBuf, mediumInstanceBuffer, mediumInstances.data(), mediumInstances.size() * sizeof(GLSLInstance));
	BLAS boxBlas;
	if (!gState.uniqueResourceCache->fetch(boxBlas, "cmdBuildBoxBlas"))
	{
		boxBlas = cmdBuildBoxBlas(cmdBuf, pPool);
		gState.uniqueResourceCache->add(boxBlas, "cmdBuildBoxBlas");
	}
	mediumTlas = createTopLevelAS(pPool, mediumInstances.size());
	default_scene::cmdBuildBoxIntersector<GLSLInstance>(cmdBuf, boxBlas, mediumInstanceBuffer, mediumInstances.size(), mediumTlas);

	resourcesTypes |= TraceResourcesType_MediumInstance;
}

void TraceResources::cmdLoadAll(CmdBuffer cmdBuf, IResourcePool *pPool)
{
	cmdLoadSceneData(cmdBuf, pPool);
	cmdLoadMedium(cmdBuf, pPool);
	cmdLoadMediumInstances(cmdBuf, pPool);
}

void TraceResources::cmdLoadUpdate(CmdBuffer cmdBuf, IResourcePool *pPool, std::vector<bool> settingsChanged)
{
	auto guiCatChanged = [settingsChanged](uint32_t setting) -> bool {
		return gvar_menu.val.v_uint == setting >> GUI_CAT_SHIFT && settingsChanged[setting & GUI_INDEX_MASK];
	};

	if (gvar_medium_load.val.v_bool || (resourcesTypes & TraceResourcesType_Medium) == 0)
	{
		cmdLoadMedium(cmdBuf, pPool);
	}
	if (guiCatChanged(GUI_CAT_SCENE_GENERAL) || guiCatChanged(GUI_CAT_SCENE_TRANSFORMS) || (resourcesTypes & TraceResourcesType_Scene) == 0)
	{
		cmdLoadSceneData(cmdBuf, pPool);
	}
	if (guiCatChanged(GUI_CAT_SCENE_MEDIUM_INSTANCES) || (resourcesTypes & TraceResourcesType_MediumInstance) == 0)
	{
		cmdLoadMediumInstances(cmdBuf, pPool);
	}

}
