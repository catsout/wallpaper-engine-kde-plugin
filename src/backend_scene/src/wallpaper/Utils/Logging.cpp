#include "Logging.h"
#include <cstdio>
#include <cstdarg>

constexpr const char* level_names[] = {
	"INFO",
	"ERROR"
};

void WallpaperLog(int level, const char *file, int line, const char *fmt, ...) {
	std::va_list args;
	std::fprintf(stderr, "%-5s %s:%d: ", level_names[level], file, line);
	{
		va_start(args, fmt);
		std::vfprintf(stderr, fmt, args);
		va_end(args);
	}
    std::fprintf(stderr, "\n");
    std::fflush(stderr); 
}