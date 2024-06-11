#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>
#include <stdarg.h>
#include <stdio.h>

#include"../core_common.h"
#include <map>
namespace vka
{
template <class integral>
constexpr integral alignUp(integral x, size_t a) noexcept
{
	return integral((x + (integral(a) - 1)) & ~integral(a - 1));
}

template <typename M, typename V>
inline void MapToVec(const M &m, V &v)
{
	v.clear();
	for (typename M::const_iterator it = m.begin(); it != m.end(); ++it)
	{
		v.push_back(it->second);
	}
}

template <class T>
inline void hashCombine(hash_t &s, const T &v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}


template <class T>
inline hash_t hashArray(const T arr[], std::size_t count)
{
	hash_t       value = 0;
	for (std::size_t i = 0; i < count; ++i)
	{
		hashCombine(value, &arr[i]);
	}
	return value;
}

template <class T>
inline hash_t shallowHashStructure(const T *v)
{
	if (v == nullptr)
	{
		return 0;
	}
	return hashArray((const uint8_t *) v, sizeof(T));
}

template <class T>
inline hash_t hashArray(const std::vector<T> &arr)
{
	hash_t value = 0;
	for (std::size_t i = 0; i < arr.size(); ++i)
	{
		hashCombine(value, arr[i]);
	}
	return value;
}

template <class T>
inline hash_t shallowHashArray(const std::vector<T> &arr)
{
	hash_t       value = 0;
	for (std::size_t i = 0; i < arr.size(); ++i)
	{
		hashCombine(value, shallowHashStructure(&arr[i]));
	}
	return value;
}

template <class T>
inline bool shallowCmpStructure(const T *a, const T *b)
{
	if (a == nullptr && b == nullptr)
	{
		return true;
	}
	if (a == nullptr || b == nullptr)
	{
		return false;
	}
	return memcmp(a, b, sizeof(T)) == 0;
}


template <class T>
inline bool shallowCmpArray(const std::vector<T> &a, const std::vector<T> &b)
{
	if (a.size() != b.size())
	{
		return false;
	}
	if (a.size() == 0)
	{
		return true;
	}
	bool isEqual = memcmp(a.data(), b.data(), a.size() * sizeof(T)) == 0;
	return isEqual;
}


template <class T>
inline bool cmpArray(const std::vector<T> &a, const std::vector<T> &b)
{
	if (a.size() != b.size())
	{
		return false;
	}
	bool isEqual = true;
	for (size_t i = 0; i < a.size(); i++)
	{
		isEqual &= a[i] == b[i];
	}
	return isEqual;
}

inline std::vector<char> readFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cout << "Failed to open a file: " << filename << std::endl;
		throw std::runtime_error("Failed to open a file!");
	}
	size_t            file_size = (size_t) file.tellg();
	std::vector<char> file_buffer(file_size);
	file.seekg(0);
	file.read(file_buffer.data(), file_size);
	file.close();
	return file_buffer;
}



inline void printVka(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

inline void printVka(const char* path, glm::mat4 mat)
{
	std::string format(path);
	format.append(": \n %.3f %.3f %.3f %.3f \n %.3f %.3f %.3f %.3f \n %.3f %.3f %.3f %.3f \n %.3f %.3f %.3f %.3f \n");
	printVka(format.c_str(),
	         mat[0].x, mat[1].x, mat[2].x, mat[3].x,
	         mat[0].y, mat[1].y, mat[2].y, mat[3].y,
	         mat[0].z, mat[1].z, mat[2].z, mat[3].z,
	         mat[0].w, mat[1].w, mat[2].w, mat[3].w);
}

inline void printVka(const char *path, glm::vec4 v)
{
	std::string format(path);
	format.append(": \n %.3f %.3f %.3f %.3f \n");
	printVka(format.c_str(), v.x, v.y, v.z, v.w);
}

inline std::vector<std::string> split(const std::string &s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string              token;
	std::istringstream       tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

template <class T>
inline bool selectByPreference(const std::vector<T> &options, const std::vector<T> &preferences, T &selection)
{
	for (size_t i = 0; i < options.size(); i++)
	{
		for (size_t j = 0; j < preferences.size(); j++)
		{
			if (std::memcmp(&options[i], &preferences[j], sizeof(T)) == 0)
			{
				selection = options[i];
				return true;
			}
		}
	}
	return false;
}

// from https://github.com/vcoda/magma
inline void *copyBinaryData(const void *src, std::size_t size) noexcept
{
	void *dst = new char[size];
	if (dst)
		memcpy(dst, src, size);
	return dst;
}

template <class T>
inline void *copyBinaryData(const T &src) noexcept
{
	void *dst = new char[sizeof(T)];
	if (dst)
		memcpy(dst, &src, sizeof(T));
	return dst;
}

constexpr void clamp(VkExtent2D &target, const VkExtent2D &minExtent, const VkExtent2D &maxExtent)
{
	target.width  = std::max(minExtent.width, std::min(maxExtent.width, target.width));
	target.height = std::max(minExtent.height, std::min(maxExtent.height, target.height));
}

inline uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	DEBUG_BREAK
	return UINT32_MAX;        // ToDo throw error message
}


template<class T>
inline uint32_t dataSize(std::vector<T> v)
{
	return static_cast<uint32_t>(v.size() * sizeof(T));
}


struct SubmitSynchronizationInfo
{
	std::vector<VkSemaphore>			waitSemaphores{};
	std::vector<VkSemaphore>			signalSemaphores{};
	std::vector<VkPipelineStageFlags>	waitDstStageMask{};
	VkFence								signalFence = VK_NULL_HANDLE;
};

inline void submit(
    std::vector<VkCommandBuffer> cmdBufs, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
{
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.waitSemaphoreCount   = syncInfo.waitSemaphores.size();
	submit.pWaitSemaphores      = syncInfo.waitSemaphores.data();
	submit.pWaitDstStageMask    = syncInfo.waitDstStageMask.data();
	submit.signalSemaphoreCount = syncInfo.signalSemaphores.size();
	submit.pSignalSemaphores    = syncInfo.signalSemaphores.data();
	submit.pCommandBuffers      = cmdBufs.data();
	submit.commandBufferCount   = cmdBufs.size();
	ASSERT_VULKAN(vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence));
}

inline void submit(VkCommandBuffer cmdBuf, VkQueue queue, const SubmitSynchronizationInfo syncInfo = {})
{
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.waitSemaphoreCount   = syncInfo.waitSemaphores.size();
	submit.pWaitSemaphores      = syncInfo.waitSemaphores.data();
	submit.pWaitDstStageMask    = syncInfo.waitDstStageMask.data();
	submit.signalSemaphoreCount = syncInfo.signalSemaphores.size();
	submit.pSignalSemaphores    = syncInfo.signalSemaphores.data();
	submit.pCommandBuffers      = &cmdBuf;
	submit.commandBufferCount   = 1;
	ASSERT_VULKAN(vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence));
}

constexpr VkExtent3D getExtent3D(const VkExtent2D extent)
{
	return {extent.width, extent.height, 1};
}


VkSpecializationInfo inline writeSpecializationInfo(
    const uint32_t mapEntriesCount,
	std::vector<VkSpecializationMapEntry> &mapEntries, const std::vector<uint32_t> &entrySizes, uint32_t &mapEntryOffset,
    const void *data, uint32_t &dataOffset)
{
	VkSpecializationInfo specInfo{};
	specInfo.pData = (char *) data + dataOffset;
	specInfo.dataSize = 0;
	specInfo.pMapEntries   = &mapEntries[mapEntryOffset];
	specInfo.mapEntryCount = mapEntriesCount;
	for (uint32_t i = 0; i < mapEntriesCount; i++)
	{
		VkSpecializationMapEntry specEntry{};
		specEntry.constantID = i;
		specEntry.offset     = specInfo.dataSize;
		specEntry.size       = entrySizes[i + mapEntryOffset];
		specInfo.dataSize += specEntry.size;
		mapEntries[i + mapEntryOffset] = specEntry;
	}
	mapEntryOffset += mapEntriesCount;
	dataOffset += specInfo.dataSize;
	return specInfo;
}

void inline writeSpecializationInfo(
    const std::vector<uint32_t>           &entrySizes,
    const void                            *data,
    std::vector<VkSpecializationMapEntry> &mapEntries,
    VkSpecializationInfo     &specInfo)
{
	mapEntries.resize(entrySizes.size());
	uint32_t mapEntryOffset = 0;
	uint32_t dataOffset     = 0;
	specInfo                = writeSpecializationInfo(entrySizes.size(), mapEntries, entrySizes, mapEntryOffset, data, dataOffset);
}

void inline writeSpecializationInfos(
    const std::vector<uint32_t> &entryCounts,
    const std::vector<uint32_t> &entrySizes,
    const void                  *data,
    std::vector<VkSpecializationMapEntry> &mapEntries,
    std::vector<VkSpecializationInfo>     &specInfos)
{
	uint32_t totalMapEntryCount = 0;
	for (size_t i = 0; i < entryCounts.size(); i++)
	{
		totalMapEntryCount += entryCounts[i];
	}
	mapEntries.resize(totalMapEntryCount);
	specInfos.resize(entryCounts.size());

	uint32_t mapEntryOffset = 0;
	uint32_t dataOffset     = 0;
	for (size_t i = 0; i < entryCounts.size(); i++)
	{
		specInfos[i] = writeSpecializationInfo(entryCounts[i], mapEntries, entrySizes, mapEntryOffset, data, dataOffset);
	}
}




glm::uvec3 inline getWorkGroupCount(const glm::uvec3 &workGroupSize, const glm::uvec3 &resolution)
{
	return {alignUp(resolution.x, workGroupSize.x) / workGroupSize.x,
	        alignUp(resolution.y, workGroupSize.y) / workGroupSize.y,
	        alignUp(resolution.z, workGroupSize.z) / workGroupSize.z};
}

std::vector<uint32_t> inline glm3VectorSizes()
{
	return {sizeof(uint32_t), sizeof(uint32_t), sizeof(uint32_t)};
}

std::vector<uint8_t> inline getByteVector(const glm::uvec3 &in)
{
	uint32_t v[] = {in.x, in.y, in.z};
	std::vector<uint8_t> out(3 * sizeof(uint32_t));
	memcpy(out.data(), v, 3 * sizeof(uint32_t));
	return out;
}

std::vector<uint8_t> inline getByteVector(void* data, size_t size)
{
	std::vector<uint8_t> out(size);
	memcpy(out.data(), data, size);
	return out;
}

// CIV
VkImageAspectFlags inline getAspectFlags(VkFormat format)
{
	VkImageAspectFlags aspectFlags = 0;
	if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D16_UNORM_S8_UINT ||
		format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	return aspectFlags;
}

VkAccessFlags inline getAccessFlags(VkImageLayout layout)
{
	switch (layout)
	{
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return VK_ACCESS_HOST_WRITE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_SHADER_READ_BIT;
		default:
			return VkAccessFlags();
	}
}

VkPipelineStageFlags inline getStageFlags(VkImageLayout oldImageLayout)
{
	switch (oldImageLayout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return VK_PIPELINE_STAGE_HOST_BIT;
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		default:
			return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
}

// format table from: https://android.googlesource.com/platform/external/vulkan-validation-layers/+/HEAD/layers/vk_format_utils.cpp
struct VULKAN_FORMAT_INFO
{
	uint32_t                   size;
	uint32_t                   channel_count;
};
// Set up data structure with size(bytes) and number of channels for each Vulkan format
// For compressed and multi-plane formats, size is bytes per compressed or shared block
const std::map<VkFormat, VULKAN_FORMAT_INFO> cVkFormatTable = {
    {VK_FORMAT_UNDEFINED, {0, 0}},
    {VK_FORMAT_R4G4_UNORM_PACK8, {1, 2}},
    {VK_FORMAT_R4G4B4A4_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_B4G4R4A4_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_R5G6B5_UNORM_PACK16, {2, 3}},
    {VK_FORMAT_B5G6R5_UNORM_PACK16, {2, 3}},
    {VK_FORMAT_R5G5B5A1_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_B5G5R5A1_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_A1R5G5B5_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_R8_UNORM, {1, 1}},
    {VK_FORMAT_R8_SNORM, {1, 1}},
    {VK_FORMAT_R8_USCALED, {1, 1}},
    {VK_FORMAT_R8_SSCALED, {1, 1}},
    {VK_FORMAT_R8_UINT, {1, 1}},
    {VK_FORMAT_R8_SINT, {1, 1}},
    {VK_FORMAT_R8_SRGB, {1, 1}},
    {VK_FORMAT_R8G8_UNORM, {2, 2}},
    {VK_FORMAT_R8G8_SNORM, {2, 2}},
    {VK_FORMAT_R8G8_USCALED, {2, 2}},
    {VK_FORMAT_R8G8_SSCALED, {2, 2}},
    {VK_FORMAT_R8G8_UINT, {2, 2}},
    {VK_FORMAT_R8G8_SINT, {2, 2}},
    {VK_FORMAT_R8G8_SRGB, {2, 2}},
    {VK_FORMAT_R8G8B8_UNORM, {3, 3}},
    {VK_FORMAT_R8G8B8_SNORM, {3, 3}},
    {VK_FORMAT_R8G8B8_USCALED, {3, 3}},
    {VK_FORMAT_R8G8B8_SSCALED, {3, 3}},
    {VK_FORMAT_R8G8B8_UINT, {3, 3}},
    {VK_FORMAT_R8G8B8_SINT, {3, 3}},
    {VK_FORMAT_R8G8B8_SRGB, {3, 3}},
    {VK_FORMAT_B8G8R8_UNORM, {3, 3}},
    {VK_FORMAT_B8G8R8_SNORM, {3, 3}},
    {VK_FORMAT_B8G8R8_USCALED, {3, 3}},
    {VK_FORMAT_B8G8R8_SSCALED, {3, 3}},
    {VK_FORMAT_B8G8R8_UINT, {3, 3}},
    {VK_FORMAT_B8G8R8_SINT, {3, 3}},
    {VK_FORMAT_B8G8R8_SRGB, {3, 3}},
    {VK_FORMAT_R8G8B8A8_UNORM, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SNORM, {4, 4}},
    {VK_FORMAT_R8G8B8A8_USCALED, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SSCALED, {4, 4}},
    {VK_FORMAT_R8G8B8A8_UINT, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SINT, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SRGB, {4, 4}},
    {VK_FORMAT_B8G8R8A8_UNORM, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SNORM, {4, 4}},
    {VK_FORMAT_B8G8R8A8_USCALED, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SSCALED, {4, 4}},
    {VK_FORMAT_B8G8R8A8_UINT, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SINT, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SRGB, {4, 4}},
    {VK_FORMAT_A8B8G8R8_UNORM_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SNORM_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_USCALED_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SSCALED_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_UINT_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SINT_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SRGB_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_UNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_SNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_USCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_SSCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_UINT_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_SINT_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_UNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_SNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_USCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_SSCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_UINT_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_SINT_PACK32, {4, 4}},
    {VK_FORMAT_R16_UNORM, {2, 1}},
    {VK_FORMAT_R16_SNORM, {2, 1}},
    {VK_FORMAT_R16_USCALED, {2, 1}},
    {VK_FORMAT_R16_SSCALED, {2, 1}},
    {VK_FORMAT_R16_UINT, {2, 1}},
    {VK_FORMAT_R16_SINT, {2, 1}},
    {VK_FORMAT_R16_SFLOAT, {2, 1}},
    {VK_FORMAT_R16G16_UNORM, {4, 2}},
    {VK_FORMAT_R16G16_SNORM, {4, 2}},
    {VK_FORMAT_R16G16_USCALED, {4, 2}},
    {VK_FORMAT_R16G16_SSCALED, {4, 2}},
    {VK_FORMAT_R16G16_UINT, {4, 2}},
    {VK_FORMAT_R16G16_SINT, {4, 2}},
    {VK_FORMAT_R16G16_SFLOAT, {4, 2}},
    {VK_FORMAT_R16G16B16_UNORM, {6, 3}},
    {VK_FORMAT_R16G16B16_SNORM, {6, 3}},
    {VK_FORMAT_R16G16B16_USCALED, {6, 3}},
    {VK_FORMAT_R16G16B16_SSCALED, {6, 3}},
    {VK_FORMAT_R16G16B16_UINT, {6, 3}},
    {VK_FORMAT_R16G16B16_SINT, {6, 3}},
    {VK_FORMAT_R16G16B16_SFLOAT, {6, 3}},
    {VK_FORMAT_R16G16B16A16_UNORM, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SNORM, {8, 4}},
    {VK_FORMAT_R16G16B16A16_USCALED, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SSCALED, {8, 4}},
    {VK_FORMAT_R16G16B16A16_UINT, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SINT, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SFLOAT, {8, 4}},
    {VK_FORMAT_R32_UINT, {4, 1}},
    {VK_FORMAT_R32_SINT, {4, 1}},
    {VK_FORMAT_R32_SFLOAT, {4, 1}},
    {VK_FORMAT_R32G32_UINT, {8, 2}},
    {VK_FORMAT_R32G32_SINT, {8, 2}},
    {VK_FORMAT_R32G32_SFLOAT, {8, 2}},
    {VK_FORMAT_R32G32B32_UINT, {12, 3}},
    {VK_FORMAT_R32G32B32_SINT, {12, 3}},
    {VK_FORMAT_R32G32B32_SFLOAT, {12, 3}},
    {VK_FORMAT_R32G32B32A32_UINT, {16, 4}},
    {VK_FORMAT_R32G32B32A32_SINT, {16, 4}},
    {VK_FORMAT_R32G32B32A32_SFLOAT, {16, 4}},
    {VK_FORMAT_R64_UINT, {8, 1}},
    {VK_FORMAT_R64_SINT, {8, 1}},
    {VK_FORMAT_R64_SFLOAT, {8, 1}},
    {VK_FORMAT_R64G64_UINT, {16, 2}},
    {VK_FORMAT_R64G64_SINT, {16, 2}},
    {VK_FORMAT_R64G64_SFLOAT, {16, 2}},
    {VK_FORMAT_R64G64B64_UINT, {24, 3}},
    {VK_FORMAT_R64G64B64_SINT, {24, 3}},
    {VK_FORMAT_R64G64B64_SFLOAT, {24, 3}},
    {VK_FORMAT_R64G64B64A64_UINT, {32, 4}},
    {VK_FORMAT_R64G64B64A64_SINT, {32, 4}},
    {VK_FORMAT_R64G64B64A64_SFLOAT, {32, 4}},
    {VK_FORMAT_B10G11R11_UFLOAT_PACK32, {4, 3}},
    {VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, {4, 3}},
    {VK_FORMAT_D16_UNORM, {2, 1}},
    {VK_FORMAT_X8_D24_UNORM_PACK32, {4, 1}},
    {VK_FORMAT_D32_SFLOAT, {4, 1}},
    {VK_FORMAT_S8_UINT, {1, 1}},
    {VK_FORMAT_D16_UNORM_S8_UINT, {3, 2}},
    {VK_FORMAT_D24_UNORM_S8_UINT, {4, 2}},
    {VK_FORMAT_D32_SFLOAT_S8_UINT, {8, 2}},
    {VK_FORMAT_BC1_RGB_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC1_RGB_SRGB_BLOCK, {8, 4}},
    {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC1_RGBA_SRGB_BLOCK, {8, 4}},
    {VK_FORMAT_BC2_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC2_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_BC3_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC3_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_BC4_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC4_SNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC5_SNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC6H_UFLOAT_BLOCK, {16, 4}},
    {VK_FORMAT_BC6H_SFLOAT_BLOCK, {16, 4}},
    {VK_FORMAT_BC7_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC7_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, {8, 3}},
    {VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, {8, 3}},
    {VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, {8, 4}},
    {VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_EAC_R11_UNORM_BLOCK, {8, 1}},
    {VK_FORMAT_EAC_R11_SNORM_BLOCK, {8, 1}},
    {VK_FORMAT_EAC_R11G11_UNORM_BLOCK, {16, 2}},
    {VK_FORMAT_EAC_R11G11_SNORM_BLOCK, {16, 2}},
    {VK_FORMAT_ASTC_4x4_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_4x4_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x4_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x4_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x6_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x6_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x6_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x6_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x8_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x8_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x6_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x6_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x8_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x8_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x10_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x10_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x10_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x10_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x12_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x12_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG, {8, 4}},
    // KHR_sampler_YCbCr_conversion extension - single-plane variants
    // 'PACK' formats are normal, uncompressed
    {VK_FORMAT_R10X6_UNORM_PACK16, {2, 1}},
    {VK_FORMAT_R10X6G10X6_UNORM_2PACK16, {4, 2}},
    {VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, {8, 4}},
    {VK_FORMAT_R12X4_UNORM_PACK16, {2, 1}},
    {VK_FORMAT_R12X4G12X4_UNORM_2PACK16, {4, 2}},
    {VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16, {8, 4}},
    // _422 formats encode 2 texels per entry with B, R components shared - treated as compressed w/ 2x1 block size
    {VK_FORMAT_G8B8G8R8_422_UNORM, {4, 4}},
    {VK_FORMAT_B8G8R8G8_422_UNORM, {4, 4}},
    {VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, {8, 4}},
    {VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, {8, 4}},
    {VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, {8, 4}},
    {VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, {8, 4}},
    {VK_FORMAT_G16B16G16R16_422_UNORM, {8, 4}},
    {VK_FORMAT_B16G16R16G16_422_UNORM, {8, 4}},
    // KHR_sampler_YCbCr_conversion extension - multi-plane variants
    // Formats that 'share' components among texels (_420 and _422), size represents total bytes for the smallest possible texel block
    // _420 share B, R components within a 2x2 texel block
    {VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, {6, 3}},
    {VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, {6, 3}},
    {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, {12, 3}},
    {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, {12, 3}},
    {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, {12, 3}},
    {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, {12, 3}},
    {VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, {12, 3}},
    {VK_FORMAT_G16_B16R16_2PLANE_420_UNORM, {12, 3}},
    // _422 share B, R components within a 2x1 texel block
    {VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, {4, 3}},
    {VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, {4, 3}},
    {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, {8, 3}},
    {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, {8, 3}},
    {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, {8, 3}},
    {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, {8, 3}},
    {VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, {8, 3}},
    {VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, {8, 3}},
    // _444 do not share
    {VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, {3, 3}},
    {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, {6, 3}},
    {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, {6, 3}},
    {VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM, {6, 3}}};

const VkExtent2D cResolution4k = {3840, 2160};
const VkExtent2D cResolution2k = {2048, 1024};



inline VkVertexInputBindingDescription getVertexBindingDescription(uint32_t size, uint32_t binding = 0)
{
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding   = binding;
	vertexInputBindingDescription.stride    = size;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return vertexInputBindingDescription;
}

inline VkVertexInputBindingDescription getInstanceBindingDescription(uint32_t size, uint32_t binding = 1)
{
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding   = binding;
	vertexInputBindingDescription.stride    = size;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	return vertexInputBindingDescription;
}

inline std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, std::vector<VkFormat> formats, std::vector<uint32_t> offsets)
{
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
	for (size_t i = 0; i < formats.size(); i++)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = i;
		attributeDescription.format   = formats[i];
		attributeDescription.offset   = offsets[i];
		attributeDescription.binding  = binding;
		vertexInputAttributeDescriptions.push_back(attributeDescription);
	}
	return vertexInputAttributeDescriptions;
}

template<typename T>
struct Rect2D
{
	T x;
	T y;
	T width;
	T height;
};

/*template <class... Args>
inline std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, VkFormat nextFormat, uint32_t nextOffset, Args... args)
{
	std::vector<VkFormat> formats = {nextFormat};
	std::vector<uint32_t> offsets = {nextOffset};
	return getAttributeDescriptions(binding, formats, offsets, args...);
}

template <class... Args>
inline std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(
    uint32_t binding, std::vector<VkFormat> &formats, std::vector<uint32_t> &offsets, VkFormat nextFormat, uint32_t nextOffset, Args... args)
{
	formats.push_back(nextFormat);
	offsets.push_back(nextOffset);
	return getAttributeDescriptions(binding, formats, offsets, args...);
}
inline std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(
    uint32_t binding, std::vector<VkFormat> &formats, std::vector<uint32_t> &offsets)
{
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
	for (size_t i = 0; i < formats.size(); i++)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = i;
		attributeDescription.format   = formats[i];
		attributeDescription.offset   = offsets[i];
		attributeDescription.binding  = binding;
		vertexInputAttributeDescriptions.push_back(attributeDescription);
	}
	return vertexInputAttributeDescriptions;
}
*/

}		// namespace vka
