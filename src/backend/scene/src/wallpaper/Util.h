#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdint>
#include "Log.h"

#define STRCONV(str,t) wallpaper::StrConv((str), (t), __FUNCTION__, __LINE__);

namespace wallpaper
{

std::vector<std::string> SpliteString(std::string str, std::string spliter);
std::string ConectVecString(const std::vector<std::string>& strs,const std::string& conector, int first, int last);

int readInt32(std::ifstream&);
float ReadFloat(std::ifstream&);
std::string readSizedString(std::ifstream&);
std::string	FileSuffix(const std::string& file);

struct LineStr
{
    std::string::size_type pos;
	std::string::size_type len; 
	std::string value;
};
LineStr GetLineWith(const std::string& src, const std::string& find, long pos=0);
bool DeleteLine(std::string& src, LineStr& line);

template<typename T>
T StrConv(const std::string& str);

template<> inline int32_t StrConv<int32_t>(const std::string& str) {
	return std::stoi(str);
}
template<> inline uint32_t StrConv<uint32_t>(const std::string& str) {
	return std::stoul(str);
}
template<> inline float StrConv<float>(const std::string& str) {
	return std::stof(str);
}

template<typename T>
bool StringToVec(const std::string& str, std::vector<T>& target) {
    std::vector<std::string> str_list = SpliteString(str, " ");
	if(target.size() < str_list.size())
		target.resize(str_list.size());
	try { 
		std::transform(str_list.begin(), str_list.end(), target.begin(), StrConv<T>);
	}
	catch (const std::invalid_argument& e) {
		std::cerr << "Invalid argument: " << e.what() << " " << str << std::endl;
		return false;
	}
	catch (const std::out_of_range& e) {
		std::cerr << "Out of range: " << e.what() << std::endl;
		return false;
	}
	return true;
}

template<typename T>
bool StrConv(const std::string& str, T& target, const char* func, int32_t line) {
	T temp;
	try { 
		temp = StrConv<T>(str);
	}
	catch (const std::invalid_argument& e) {
		Logger::Log(func, line, "Error converting string at ", std::string("invalid argument ") + e.what());
		return false;
	}
	target = temp;
	return true;
}
}