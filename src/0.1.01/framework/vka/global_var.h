#pragma once
#include "common.h"
// Gui Variable
// Value

namespace vka
{

union GVar_Val
{
	bool     v_bool;
	uint32_t v_uint;
	int      v_int;
	float    v_float;
	float    v_vec3[3];

	GVar_Val(bool b)
	{
		v_bool = b;
	}
	GVar_Val(uint32_t i)
	{
		v_uint = i;
	}
	GVar_Val(int i)
	{
		v_int = i;
	}
	GVar_Val(float f)
	{
		v_float = f;
	}
	GVar_Val(float x, float y, float z)
	{
		v_vec3[0] = x;
		v_vec3[1] = y;
		v_vec3[2] = z;
	}
	glm::vec4 getVec4()
	{
		return glm::vec4(v_vec3[0], v_vec3[1], v_vec3[2], 0);
	};
	uint32_t bool32()
	{
		return v_bool ? 1 : 0;
	};
};

typedef std::vector<std::string> GVar_Enum_t;

// Type
enum GVar_Type
{
	GVAR_EVENT,
	GVAR_BOOL,
	GVAR_FLOAT,
	GVAR_UNORM,
	GVAR_INT,
	GVAR_UINT,
	GVAR_VEC3,
	GVAR_DISPLAY_VALUE,
	GVAR_ENUM,
};

// Categorie
enum GVar_Cat
{
	GVAR_FRAMEWORK,
	GVAR_APPLICATION,
};

struct GVar
{
	std::string path;
	GVar_Val    val;
	GVar_Type   type;
	GVar_Cat    cat;
	GVar_Enum_t enumVal;
};
}		// namespace vka