#include "Log.h"
#include <iostream>

namespace wp = wallpaper;

void wp::Logger::Log(const char* func, int line, const char* prefix, const char* text) {
	Log(func, line, prefix, std::string(text));
}

void wp::Logger::Log(const char* func, int line, const char* prefix, const std::string& text) {
	std::string linestr, prefixEnd;
	if(line != -1)
		linestr = '(' + std::to_string(line) + ')';
	if(*prefix != '\0')
		prefixEnd = ": ";
	std::cerr << prefix << func << linestr << prefixEnd << text << std::endl;
}

