#pragma once
#include <string>

#define LOG_ERROR(text) wallpaper::Logger::Log(__FUNCTION__, __LINE__, "Error at ", (text));
#define LOG_INFO(text) wallpaper::Logger::Log("", -1, "", (text));

namespace wallpaper
{

class Logger {
public:
	static void Log(const char* func, int line, const char* prefix, const std::string& text);
	static void Log(const char* func, int line, const char* prefix, const char* text);
};
}