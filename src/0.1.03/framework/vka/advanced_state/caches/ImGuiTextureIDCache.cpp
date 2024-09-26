#include "ImGuiTextureIDCache.h"
#include <imgui_impl_vulkan.h>
namespace vka
{
//void ImGuiTextureIDCache::clear()
//{
//	for (auto &pair : map)
//	{
//		ImGui_ImplVulkan_SetMinImageCount(pair.second);
//	}
//	map.clear();
//}
//bool ImGuiTextureIDCache::fetch(Buffer &buf, hash_t key)
//{
//	if (map.find(key) != map.end())
//	{
//		buf = map[key];
//		return true;
//	}
//	else
//	{
//		buf      = createBuffer(pPool, 0);
//		map[key] = buf;
//		return false;
//	}
//}

}        // namespace vka