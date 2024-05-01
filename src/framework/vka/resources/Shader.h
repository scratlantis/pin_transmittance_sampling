#pragma once
#include "Resource.h"
namespace vka
{
struct ShaderArgs
{
	std::string name;
	std::string value;

	bool operator==(const ShaderArgs &other) const
	{
		return name == other.name && value == other.value;
	}
};





struct ShaderDefinition
{
	std::string name;
	std::vector<ShaderArgs> args;

	bool operator==(const ShaderDefinition &other) const
	{
		bool isEqual = name == other.name &&cmpArray(args, other.args);
		return isEqual;
	}
	bool operator!=(const ShaderDefinition &other) const
	{
		return !(*this == other);
	}

	std::string fileID() const
	{
		std::string id = name;
		for (size_t i = 0; i < args.size(); i++)
		{
			id.append("_");
			id.append(args[i].name);
			if (args[i].value != "")
			{
				id.append("=");
			}
			id.append(args[i].value);
		}
		return id;
	}


	hash_t hash() const
	{
		return std::hash<std::string>()(fileID());
	};

	std::string fileIDShort() const
	{
		std::string id = name;
		id.append(std::to_string(hash()));
		return id;
	}


};

#define VKA_NULL_SHADER \
	ShaderDefinition    \
	{                   \
		"",             \
		{}              \
	}

class Shader : public UniqueResource<VkShaderModule>
{
  protected:
	void free()
	{
		vkDestroyShaderModule(gState.device.logical, handle, nullptr);
	}
	void buildHandle()
	{
		createModule();
	}
	//virtual bool _equals(Shader const &other) const
	//{
	//	return def == other.def;
	//}

	

	virtual bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		auto that = static_cast<Shader const &>(other);
		return *this == that;
	}

	Shader *copyToHeap() const
	{
		return new Shader(*this);
	}

  public:
	Shader(ResourceTracker *pTracker, ShaderDefinition shaderDefinition) :
	    UniqueResource(pTracker), def(shaderDefinition)
	{
		ASSERT_TRUE(def != VKA_NULL_SHADER)
	}
	hash_t _hash() const
	{
		return def.hash();
	};

	ShaderDefinition        def;

	std::string fileID() const
	{
		return def.fileID();
	}

	std::string fileIDShort() const
	{
		return def.fileIDShort();
	}

	bool operator==(const Shader &other) const
	{
		return def == other.def;
	}

	VkPipelineShaderStageCreateInfo getStageCI()
	{
		VkPipelineShaderStageCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		ci.module          = getHandle();
		ci.pName           = "main";
		std::string suffix = def.name.substr(def.name.find_last_of(".") + 1);
		if (suffix == "vert")
		{
			ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (suffix == "frag")
		{
			ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		else if (suffix == "geom")
		{
			ci.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		}
		else if (suffix == "comp")
		{
			ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		else if (suffix == "rgen")
		{
			ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		}
		else if (suffix == "rmiss")
		{
			ci.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		}
		else if (suffix == "rahit")
		{
			ci.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		}
		else if (suffix == "rchit")
		{
			ci.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		}
		else
		{
			DEBUG_BREAK;
		}
		return ci;
	}

	private:
	VkShaderModule shaderModule = VK_NULL_HANDLE;
	void compile()
	{
		std::stringstream shader_src_path;
		std::stringstream shader_spv_path;
		std::stringstream shader_log_path;
		std::stringstream cmdShaderCompile;
		shader_src_path << gShaderPath << "/" << def.name;
		shader_spv_path << gShaderPath << "/spv/" << fileIDShort() << ".spv";
		shader_log_path << gShaderPath << "/log/" << fileIDShort() << ".log.txt";

		cmdShaderCompile << GLSLC_COMMAND;
		for (size_t i = 0; i < def.args.size(); i++)
		{
			cmdShaderCompile << " -D" << def.args[i].name << "=" << def.args[i].value;
		}
		std::string suffix = def.name.substr(def.name.find_last_of(".") + 1);
		std::string prefix = def.name.substr(0, def.name.find_first_of("."));
		for (auto &c : suffix)
			c = toupper(c);
		for (auto &c : prefix)
			c = toupper(c);
		cmdShaderCompile << " -D" << prefix << "_" << suffix;

		cmdShaderCompile << " -o " << shader_spv_path.str() << " " << shader_src_path.str()
		                 << " 2> " << shader_log_path.str();

		system(cmdShaderCompile.str().c_str());
	}

	void createModule()
	{
		std::vector<char> shader_log;
		compile();
		std::stringstream shader_log_path;
		shader_log_path << gShaderPath << "/log/" << fileIDShort() << ".log.txt";
		std::string shader_log_path_str = shader_log_path.str();
		printVka("About to open file %s\n", shader_log_path_str.c_str());
		shader_log = readFile(shader_log_path_str);
		printVka("Succes!\n");
		if (shader_log.size() > 0)
		{
			printVka("Error compiling shader '%s' : %s", fileIDShort().c_str(), shader_log.data());
		}

		std::stringstream shader_spv_path;
		shader_spv_path << gShaderPath << "/spv/" << fileIDShort() << ".spv";
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
		ASSERT_VULKAN(vkCreateShaderModule(gState.device.logical, &shaderCreateInfo, nullptr, &handle));
	}
};
}        // namespace vka

namespace std
{
template <>
struct hash<vka::Shader>
{
	size_t operator()(vka::Shader const &shader) const
	{
		return shader._hash();
	}
};

template <>
struct hash<vka::ShaderDefinition>
{
	size_t operator()(vka::ShaderDefinition const &shaderDef) const
	{
		return shaderDef.hash();
	}
};
}        // namespace std
