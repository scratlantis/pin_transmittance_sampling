#include "pt_resources.h"
#include "ui.h"
#include <random>


extern GVar gvar_medium_density_scale;
extern GVar gvar_menu;

// UScene
GVar gvar_model_path{"Select model", modelPath + "under_the_c/scene_1.obj", GVAR_FILE_INPUT, GUI_CAT_SCENE_GENERAL, std::vector<std::string>({".obj", modelPath})};
GVar gvar_texture_path{"Select envmap", texturePath + "envmap/2k/cloudy_dusky_sky_dome_2k.hdr", GVAR_FILE_INPUT, GUI_CAT_SCENE_GENERAL, std::vector<std::string>({".hdr", texturePath})};

VkExtent3D gMedTextureSize = VkExtent3D{0, 0, 0};
VkDeviceSize gMedTextureMemSize = 0;


// Medium
enum MediumSource
{
	MEDIUM_SOURCE_FILE,
	MEDIUM_SOURCE_NOISE,
	MEDIUM_SOURCE_FILE_PLUS_NOISE,
	MEDIUM_SOURCE_FILE_TIMES_NOISE,
	MEDIUM_SOURCE_FILE_MINUS_NOISE,
};
GVar gvar_medium_source{"Medium Source", 0U, GVAR_ENUM, GUI_CAT_SCENE_MEDIUM, std::vector<std::string>({"File", "Perlin Noise", "File + Noise", "File x Noise","File - Noise" })};
// File
GVar gvar_medium_path{"Select medium", texturePath + "csafe_heptane_302x302x302_uint8.raw", GVAR_FILE_INPUT, GUI_CAT_SCENE_MEDIUM, std::vector<std::string>({".raw", scalarFieldPath})};
GVar gvar_medium_load{"Load medium", false, GVAR_EVENT, GUI_CAT_SCENE_MEDIUM};
GVar gvar_medium_minVal{"Medium min val", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM, {0.0f, 1.f}};
// Noise
std::vector<uvec3> noiseResolution = {uvec3(64), uvec3(128), uvec3(256), uvec3(512), uvec3(1024)};
GVar               gvar_medium_noise_resolution{"Noise resolution", 0U, GVAR_ENUM, GUI_CAT_SCENE_MEDIUM, std::vector<std::string>({"64x64x64", "128x128x128", "256x256x256", "512x512x512", "1024x1024x1024"})};
GVar gvar_medium_noise_seed{"Noise seed", 42U, GVAR_UINT_RANGE, GUI_CAT_SCENE_MEDIUM, {0, 1000}};
GVar gvar_medium_noise_scale{"Noise scale", 0.2f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM, {0.01f, 100.f}};
GVar gvar_medium_noise_min{"Noise min", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM, {0.0f, 1.f}};
GVar gvar_medium_noise_max{"Noise max", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM, {0.0f, 1.f}};
GVar gvar_medium_noise_frequency{"Noise frequency", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM, {0.0f, 100.f}};
GVar gvar_medium_noise_falloff{"Noise falloff", 0.5f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM, {0.0f, 1.f}};
GVar gvar_medium_noise_blend_coef{"Blend coef", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM, {0.0f, 10.f}};

enum MediumDataType
{
	MEDIUM_DATA_TYPE_16BIT,
	MEDIUM_DATA_TYPE_32BIT,
};

GVar gvar_medium_data_type{"Medium data type", 0U, GVAR_ENUM, GUI_CAT_SCENE_MEDIUM, std::vector<std::string>({"16bit", "32bit"})};

// Medium Instances
GVar gvar_medium_instance_shader_path{"Select medium instance shader", shaderPath + "medium/gen_inst_default.comp", GVAR_FILE_INPUT, GUI_CAT_SCENE_MEDIUM_INSTANCE, std::vector<std::string>({".comp", shaderPath})};
GVar gvar_medium_instamce_shader_params{"MedInst. current params:", std::string(""), GVAR_DISPLAY_TEXT, GUI_CAT_SCENE_MEDIUM_INSTANCE};
GVar gvar_medium_instamce_shader_params_input{"MedInst. s. params", std::string(""), GVAR_TEXT_INPUT, GUI_CAT_SCENE_MEDIUM_INSTANCE};
GVar gvar_medium_inst_update{"Update", false, GVAR_EVENT, GUI_CAT_SCENE_MEDIUM_INSTANCE};

// Medium Instance Args
GVar gvar_medium_inst_count{"Instance count", 50, GVAR_UINT_RANGE, GUI_CAT_SCENE_MEDIUM_INSTANCE_ARGS, {1, 1000}};
GVar gvar_medium_inst_seed{"Instance seed", 42, GVAR_UINT_RANGE, GUI_CAT_SCENE_MEDIUM_INSTANCE_ARGS, {0, 1000}};
GVar gvar_medium_inst_scale_min{"Instance scale min", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM_INSTANCE_ARGS, {0.01f, 10.f}};
GVar gvar_medium_inst_scale_max{"Instance scale max", 0.9f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM_INSTANCE_ARGS, {0.01f, 10.f}};
GVar gvar_medium_inst_up{"Instance up", 0U, GVAR_ENUM, GUI_CAT_SCENE_MEDIUM_INSTANCE_ARGS, std::vector<std::string>({"A", "B", "C"})};

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
	info.minVal = gvar_medium_minVal.val.v_float;
	return info;
}

const std::string default_env_map_path           = texturePath + "envmap/2k/cloudy_dusky_sky_dome_2k.hdr";
const glm::uvec2  env_map_pdf_resolution = glm::uvec2(64, 64);

const std::string default_model_path = modelPath + "under_the_c/scene_1.obj";

const std::string default_medium_path   = scalarFieldPath + "csafe_heptane_302x302x302_uint8.raw";

const std::string default_medium_instance_shader_path = shaderPath + "medium/gen_inst_default.comp";

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

extern GVar gvar_show_tex_size;

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



	VkFormat format;
	if (gvar_medium_data_type.val.v_bool == MEDIUM_DATA_TYPE_16BIT)
	{
		format = VK_FORMAT_R16_SFLOAT;
	}
	else
	{
		format = VK_FORMAT_R32_SFLOAT;
	}

	if (gvar_medium_source.val.v_uint == MEDIUM_SOURCE_FILE)
	{
		Buffer scalarBuf;
		gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, medium_path, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		mediumTexture = createImage3D(pPool, format, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		cmdLoadScalarField(cmdBuf, scalarBuf, mediumTexture, getScalarFieldInfo(medium_path));
	}
	else if (gvar_medium_source.val.v_uint == MEDIUM_SOURCE_NOISE)
	{
		uvec3 &res    = noiseResolution[gvar_medium_noise_resolution.val.v_uint];
		mediumTexture = createImage(pPool, format, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VkExtent3D{res.x, res.y, res.z});
		PerlinNoiseArgs args{};
		args.seed          = gvar_medium_noise_seed.val.v_uint;
		args.scale         = gvar_medium_noise_scale.val.v_float;
		args.min           = gvar_medium_noise_min.val.v_float;
		args.max           = gvar_medium_noise_max.val.v_float;
		args.frequency     = gvar_medium_noise_frequency.val.v_float;
		args.falloffAtEdge = gvar_medium_noise_falloff.val.v_float;
		args.blendCoef     = 0.0f;
		getCmdPerlinNoise(mediumTexture, args).exec(cmdBuf);
	}
	else
	{
		Buffer scalarBuf;
		gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, medium_path, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		mediumTexture = createImage3D(pPool, format, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		cmdLoadScalarField(cmdBuf, scalarBuf, mediumTexture, getScalarFieldInfo(medium_path));
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT);
		PerlinNoiseArgs args{};
		args.seed          = gvar_medium_noise_seed.val.v_uint;
		args.scale         = gvar_medium_noise_scale.val.v_float;
		args.min           = gvar_medium_noise_min.val.v_float;
		args.max           = gvar_medium_noise_max.val.v_float;
		args.frequency     = gvar_medium_noise_frequency.val.v_float;
		args.falloffAtEdge = gvar_medium_noise_falloff.val.v_float;
		args.blendCoef     = gvar_medium_noise_blend_coef.val.v_float;
		if (gvar_medium_source.val.v_uint == MEDIUM_SOURCE_FILE_PLUS_NOISE) args.blendMode = 1;
		if (gvar_medium_source.val.v_uint == MEDIUM_SOURCE_FILE_TIMES_NOISE) args.blendMode = 2;
		if (gvar_medium_source.val.v_uint == MEDIUM_SOURCE_FILE_MINUS_NOISE) args.blendMode = 3;
		getCmdPerlinNoise(mediumTexture, args).exec(cmdBuf);
	}
	gvar_show_tex_size.val.v_float = mediumTexture->getMemorySize() / (1024.0f * 1024.0f);
	gMedTextureSize = mediumTexture->getExtent();
	gMedTextureMemSize = mediumTexture->getMemorySize();
	resourcesTypes |= TraceResourcesType_Medium;
}

struct MediumInstaceArgs
{
	glm::vec2     scale;
	uint32_t	  seed;
	uint32_t	  count;
	uint32_t	  up;

};

ComputeCmd getCmdGenerateMediumInstances(Buffer instanceBuffer, MediumInstaceArgs args, ShaderDefinition shader)
{
	ComputeCmd cmd(args.count, shader.path, shader.args);
	instanceBuffer->changeSize(args.count * sizeof(GLSLInstance));
	instanceBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	instanceBuffer->recreate();
	cmd.pushDescriptor(instanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushConstant(&args, sizeof(args));
	return cmd;
}

std::vector<ShaderArgs> parseShaderArgs(std::vector<char> &args)
{
	std::vector<ShaderArgs> shaderArgs;
	std::string             name = "";
	std::string             value = "";
	bool                    inName = true;
	for (char c : args)
	{
		std::string &target = inName ? name : value;
		if ((c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_') && inName)
		{
			target += toupper(c);
		}
		else if ((c >= '0' && c <= '9' || c == '.') && !inName)
		{
			target += c;
		}
		else if (c == '=')
		{
			inName = false;
		}
		else if (c == ';')
		{
			shaderArgs.push_back({name, value});
			name = "";
			value = "";
			inName = true;
		}
	}
	return shaderArgs;
}



void TraceResources::cmdLoadMediumInstances(CmdBuffer cmdBuf, IResourcePool *pPool, IResourceCache* pCache)
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


	ShaderDefinition shaderDef{};
	shaderDef.path = medium_instance_shader_path;
	shaderDef.args = parseShaderArgs(gvar_medium_instamce_shader_params.val.v_char_array);
	MediumInstaceArgs args{};
	args.scale = glm::vec2(gvar_medium_inst_scale_min.val.v_float, gvar_medium_inst_scale_max.val.v_float);
	args.seed  = gvar_medium_inst_seed.val.v_uint;
	args.count = gvar_medium_inst_count.val.v_uint;
	args.up    = gvar_medium_inst_up.val.v_uint;

	IResourceCache *tmp = gState.cache;
	gState.cache = pCache;
	getCmdGenerateMediumInstances(mediumInstanceBuffer, args, shaderDef).exec(cmdBuf);
	gState.cache = tmp;


	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	//std::vector<GLSLInstance> mediumInstances = generateInstanceDistributionV1(medium_instance_count, 42);
	//cmdWriteCopy(cmdBuf, mediumInstanceBuffer, mediumInstances.data(), mediumInstances.size() * sizeof(GLSLInstance));
	BLAS boxBlas;
	if (!gState.uniqueResourceCache->fetch(boxBlas, "cmdBuildBoxBlas"))
	{
		boxBlas = cmdBuildBoxBlas(cmdBuf, pPool);
		gState.uniqueResourceCache->add(boxBlas, "cmdBuildBoxBlas");
	}
	mediumTlas = createTopLevelAS(pPool, args.count);
	default_scene::cmdBuildBoxIntersector<GLSLInstance>(cmdBuf, boxBlas, mediumInstanceBuffer, args.count, mediumTlas);

	resourcesTypes |= TraceResourcesType_MediumInstance;
}

void TraceResources::cmdLoadAll(CmdBuffer cmdBuf, IResourcePool *pPool, IResourceCache *pCache)
{
	cmdLoadSceneData(cmdBuf, pPool);
	cmdLoadMedium(cmdBuf, pPool);
	cmdLoadMediumInstances(cmdBuf, pPool, pCache);
}

void TraceResources::cmdLoadUpdate(CmdBuffer cmdBuf, IResourcePool *pPool, IResourceCache *pCache, std::vector<bool> settingsChanged)
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
	if (guiCatChanged(GUI_CAT_SCENE_MEDIUM_INSTANCE_ARGS) || gvar_medium_inst_update.val.v_bool || (resourcesTypes & TraceResourcesType_MediumInstance) == 0)
	{
		cmdLoadMediumInstances(cmdBuf, pPool, pCache);
	}

}
