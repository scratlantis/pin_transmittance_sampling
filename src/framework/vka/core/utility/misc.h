#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>

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
	std::hash<T> h;
	hash_t       value = 0;
	for (std::size_t i = 0; i < count; ++i)
	{
		hashCombine(value, arr[i]);
	}
	return value;
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


}        // namespace vka