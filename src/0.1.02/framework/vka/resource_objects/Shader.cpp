#include "Shader.h"
#include <filesystem>
namespace vka
{
// Overrides start
hash_t ShaderDefinition::hash() const
{
	return std::hash<std::string>()(fileID());
}

bool ShaderDefinition::_equals(ResourceIdentifier const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<ShaderDefinition const &>(other);
		return this->equals(other_);
	}
}
bool ShaderDefinition::equals(ShaderDefinition const &other) const
{
	return path == other.path && cmpVector(args, other.args);
}

std::string ShaderDefinition::fileID() const
{
	std::string id = path;
	for (size_t i = 0; i < args.size(); i++)
	{
		id.append("_");
		id.append(args[i].path);
		if (args[i].value != "")
		{
			id.append("=");
		}
		id.append(args[i].value);
	}
	return id;
}

std::string ShaderDefinition::fileIDShort() const
{
	std::string name   = path.substr(path.find_last_of("/") + 1);
	name               = name.substr(0, name.find_last_of("."));
	std::string suffix = path.substr(path.find_last_of(".") + 1);
	std::string id     = name + "_" + std::to_string(hash()) + "_" + suffix;
	return id;
}

hash_t Shader::hash() const
{
	return (hash_t) handle;
}

bool Shader::_equals(Resource const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<Shader const &>(other);
		return this->handle == other_.handle;
	}
}
void Shader::free()
{
	vkDestroyShaderModule(gState.device.logical, handle, nullptr);
}
// Overrides end

VkShaderModule Shader::getHandle() const
{
	return handle;
}

Shader::Shader(ShaderDefinition const &definition)
{
	createModule(definition);
}

void Shader::compile(ShaderDefinition const &def)
{
	std::stringstream shader_src_path;
	std::stringstream shader_spv_path;
	std::stringstream shader_log_path;
	std::stringstream cmdShaderCompile;
	shader_src_path << def.path;

	shader_spv_path << gShaderOutputDir << "/spv";
	std::filesystem::create_directories(shader_spv_path.str());
	shader_spv_path << "/" << def.fileIDShort() << ".spv";

	shader_log_path << gShaderOutputDir << "/log";
	std::filesystem::create_directories(shader_log_path.str());
	shader_log_path << "/" << def.fileIDShort() << "_log.txt";

	cmdShaderCompile << GLSLC_COMMAND;
	for (size_t i = 0; i < def.args.size(); i++)
	{
		cmdShaderCompile << " -D" << def.args[i].path << "=" << def.args[i].value;
	}
	std::string suffix = def.path.substr(def.path.find_last_of(".") + 1);
	std::string name   = def.path.substr(0, def.path.find_last_of("."));
	name               = name.substr(name.find_last_of("/") + 1);
	for (auto &c : suffix)
		c = toupper(c);
	for (auto &c : name)
		c = toupper(c);
	cmdShaderCompile << " -D" << name << "_" << suffix;

	cmdShaderCompile << " -o " << shader_spv_path.str() << " " << shader_src_path.str()
	                 << " 2> " << shader_log_path.str();
	// printVka(cmdShaderCompile.str().c_str());
	system(cmdShaderCompile.str().c_str());
}

void Shader::createModule(ShaderDefinition const &def)
{
	std::vector<char> shader_log;
	compile(def);
	std::stringstream shader_log_path;
	shader_log_path << gShaderOutputDir << "/log/" << def.fileIDShort() << "_log.txt";
	std::string shader_log_path_str = shader_log_path.str();
	printVka("About to open file %s\n", shader_log_path_str.c_str());
	shader_log = readFile(shader_log_path_str);
	printVka("Succes!\n");
	if (shader_log.size() > 0)
	{
		printVka("Error compiling shader '%s' : %s", def.fileIDShort().c_str(), shader_log.data());
	}

	std::stringstream shader_spv_path;
	shader_spv_path << gShaderOutputDir << "/spv/" << def.fileIDShort() << ".spv";
	std::string shader_spv_path_str = shader_spv_path.str();
	printVka("About to open file %s\n", shader_spv_path_str.c_str());
	auto shaderCode = readFile(shader_spv_path_str.c_str());
	if (shaderCode.size() == 0)
	{
		printVka("Shader not found : %s", shader_spv_path_str.c_str());
		DEBUG_BREAK
	}
	else
	{
		printVka("Succes!\n");
	}
	VkShaderModuleCreateInfo shaderCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	shaderCreateInfo.codeSize = shaderCode.size();
	shaderCreateInfo.pCode    = (uint32_t *) shaderCode.data();
	VK_CHECK(vkCreateShaderModule(gState.device.logical, &shaderCreateInfo, nullptr, &handle));
}


}        // namespace vka