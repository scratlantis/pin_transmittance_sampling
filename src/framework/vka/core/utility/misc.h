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
	return memcmp(a, b, sizeof(T));
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

inline void submit(VkCommandBuffer cmdBuf, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
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


uint32_t inline writeSpecializationInfo(const uint32_t *pEntrySizes, uint32_t entrySizesCount,
                                                    const void *data, VkSpecializationInfo &specInfo)
{
	std::vector<VkSpecializationMapEntry> specEntries;
	uint32_t                              offset = 0;
	for (uint32_t i = 0; i < entrySizesCount; i++)
	{
		VkSpecializationMapEntry specEntry;
		specEntry.constantID = i;
		specEntry.offset     = offset;
		specEntry.size       = pEntrySizes[i];
		offset += specEntry.size;
	}
	specInfo.mapEntryCount = specEntries.size();
	specInfo.pMapEntries   = specEntries.data();
	specInfo.dataSize      = offset;
	specInfo.pData = data;

	return offset;
}
void inline writeSpecializationInfo(
    const std::vector<uint32_t> &entryCounts, const std::vector<uint32_t> &entrySizes,
	const void *data, std::vector<VkSpecializationInfo> &specInfo)
{
	specInfo.resize(entryCounts.size());
	char* dataPtr = (char*)data;
	uint32_t sizesOffset = 0;
	for (size_t i = 0; i < entryCounts.size(); i++)
	{
		if (entryCounts[i] == 0)
		{
			continue;
		}
		specInfo[i] = {};
		uint32_t offset = writeSpecializationInfo(&entrySizes[sizesOffset], entryCounts[i], dataPtr, specInfo[i]);
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
	std::vector<uint32_t> v = {in.x,
	                           in.y,
	                           in.z};
	return std::vector<uint8_t>(v.begin(), v.end());
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

}		// namespace vka