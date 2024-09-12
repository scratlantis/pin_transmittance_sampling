#pragma once
#include <vka/vka.h>
#include <filesystem>
struct ModelInfo
{
	std::string path;
	glm::vec3   offset;
	float       scale;
	glm::vec3       rotation;

	glm::mat4 getObjToWorldMatrix()
	{
		glm::mat4 objToWorld = glm::mat4(1.0f);
		objToWorld           = glm::translate(objToWorld, offset);
		objToWorld           = glm::rotate(objToWorld, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		objToWorld           = glm::rotate(objToWorld, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		objToWorld           = glm::rotate(objToWorld, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		objToWorld           = glm::scale(objToWorld, glm::vec3(scale));
		return objToWorld;
	};
};

void addFileNamesToEnum(std::string path, std::vector<std::string> &enumVal)
{
	for (const auto &entry : std::filesystem::directory_iterator(path))
	{
		std::string name = entry.path().string();
		name             = name.substr(name.find_last_of("/") + 1);
		enumVal.push_back(name);
	}
}
//// Scan envmap
//if (gvar_env_map.val.v_uint != envMapIndexLastFrame || gvar_reload.val.v_bool || cnt == 0)
//{
//	gvar_env_map.enumVal = {"None"};
//	for (const auto &entry : std::filesystem::directory_iterator(texturePath + std::string("envmap/2k/")))
//	{
//		std::string name = entry.path().string();
//		name             = std::string("envmap/2k/") + name.substr(name.find_last_of("/") + 1);
//		gvar_env_map.enumVal.push_back(name);
//	}
//	gvar_env_map.val.v_uint = std::min(gvar_env_map.val.v_uint, uint32_t(gvar_env_map.enumVal.size() - 1));
//
//}