#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>


#define GLCALL(x) x\
    wallpaper::gl::checkGlError(__FILE__, __FUNCTION__, __LINE__);

#define LOG_ERROR(text) wallpaper::Logger::Log(__FUNCTION__, __LINE__, "Error at ", (text));
#define LOG_INFO(text) wallpaper::Logger::Log("", -1, "", (text));

#if defined(DEBUG_OPENGL)
#define CHECK_GL_ERROR_IF_DEBUG() wallpaper::gl::checkGlError(__FILE__, __FUNCTION__, __LINE__);
#else
#define CHECK_GL_ERROR_IF_DEBUG()
#endif

namespace wallpaper
{
namespace gl
{
void checkGlError(const char* file, const char* func, int line);
}
class Logger {
public:
	static void Log(const char* func, int line, const char* prefix, const std::string& text);
	static void Log(const char* func, int line, const char* prefix, const char* text);
};


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
template<> inline int StrConv<int>(const std::string& str) {
	return std::stoi(str);
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
		std::cerr << "Invalid argument: " << e.what() << std::endl;
		return false;
	}
	catch (const std::out_of_range& e) {
		std::cerr << "Out of range: " << e.what() << std::endl;
		return false;
	}
	return true;
}
}
