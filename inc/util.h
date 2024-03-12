

#ifndef _UTIL_H
#define _UTIL_H

#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>

namespace simlink
{
	inline bool map_contains(std::unordered_map<std::string, std::string> map, std::string key)
	{
		return map.find(key) != map.end();
	}

	inline bool map_contains(std::unordered_map<std::string, std::pair<std::string, std::string>> map, std::string key)
	{
		return map.find(key) != map.end();
	}

	inline bool map_contains(
		std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> map,
		std::string key)
	{

		return map.find(key) != map.end();
	}

	inline bool file_exists(const std::string &name)
	{
		std::ifstream f(name.c_str());
		bool good = f.good();
		f.close();
		return good;
	}
} // namespace simlink

#endif