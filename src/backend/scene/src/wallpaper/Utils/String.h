#pragma once
#include <algorithm>
#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include "Logging.h"

#define STRTONUM(s, num) wallpaper::utils::StrToNum(s, num, __FILE__, __LINE__);

namespace wallpaper
{
namespace utils
{

template<typename TNum>
void _StrToNum(std::string_view s, TNum& num) {
}
template<> inline void _StrToNum<int32_t>(std::string_view s, int32_t& num) {
	num = std::stoi(std::string(s));
}
template<> inline void _StrToNum<uint32_t>(std::string_view s, uint32_t& num) {
	num = std::stoul(std::string(s));
}
template<> inline void _StrToNum<float>(std::string_view s, float& num) {
	num = std::stof(std::string(s));
}


template<typename TNum>
void StrToNum(std::string_view s, TNum& num, const char* file, int line) {
	try { 
		_StrToNum(s, num);
	}
	catch (const std::invalid_argument& e) {
		WallpaperLog(LOGLEVEL_ERROR, file, line, "not a number");
	}
	catch (const std::out_of_range& e) {
		WallpaperLog(LOGLEVEL_ERROR, file, line, "too larger number");
	}
}


inline std::vector<std::string> SpliteString(std::string str, char spliter) {
    std::vector<std::string> result;
    std::size_t pos = 0;
    while((pos = str.find_first_of(spliter)) != std::string::npos) {
        result.push_back(str.substr(0, pos));
        str = str.substr(pos+1);
    }
    result.push_back(str);
    return result;
}

template<typename T>
bool StringToVec(const std::string& str, std::vector<T>& target) {
    std::vector<std::string> str_list = SpliteString(str, ' ');
	if(target.size() < str_list.size())
		target.resize(str_list.size());
	auto StrConv = [](std::string str) {
		T num {};
		STRTONUM(str, num);
		return num;
	};
	std::transform(str_list.begin(), str_list.end(), target.begin(), StrConv);
	return true;
}

}
}
