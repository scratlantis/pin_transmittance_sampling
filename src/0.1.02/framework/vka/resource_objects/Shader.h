#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>

namespace vka
{


struct ShaderArgs
{
	std::string path;
	std::string value;

	bool operator==(const ShaderArgs &other) const
	{
		return path == other.path && value == other.value;
	}
};



class ShaderDefinition : public ResourceIdentifier
{
  public:
	std::string             path;
	std::vector<ShaderArgs> args;

	hash_t hash() const;
	bool   _equals(ResourceIdentifier const &other) const override;
	bool   equals(ShaderDefinition const &other) const;

	std::string fileID() const;
	std::string fileIDShort() const;
  protected:
};

class Shader : public CachableResource
{
  private:
	VkShaderModule handle;
	void           compile(ShaderDefinition const &def);
	void           createModule(ShaderDefinition const &def);
  public:
	virtual bool     _equals(Resource const &other) const override;
	virtual hash_t   hash() const override;
	virtual void     free() override;
	VkShaderModule getHandle() const;
	Shader(ShaderDefinition const &definition);
};


static VkPipelineShaderStageCreateInfo makeShaderStageCI(const ShaderDefinition const &def, const Shader *shader)
{
	{
		VkPipelineShaderStageCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		ci.module          = shader->getHandle();
		ci.pName           = "main";
		std::string suffix = def.path.substr(def.path.find_last_of(".") + 1);
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
}
}        // namespace vka