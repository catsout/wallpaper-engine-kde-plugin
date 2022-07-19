#include "Logging.h"
#include <cstdio>
#include <cstdarg>
#include <filesystem>

#include "Sha.hpp"

constexpr const char* level_names[] = { "INFO", "ERROR" };
constexpr const char* level_fmt[]   = { "%-5s", "%-5s %s:%d " };

void WallpaperLog(int level, const char* file, int line, const char* fmt, ...) {
    std::va_list args;
    std::fprintf(stderr, level_fmt[level], level_names[level], file, line);
    {
        va_start(args, fmt);
        std::vfprintf(stderr, fmt, args);
        va_end(args);
    }
    std::fprintf(stderr, "\n");
    std::fflush(stderr);
}

std::string logToTmpfileWithSha1(std::span<const char> in, const char* fmt, ...) {
    std::va_list          args;
    std::string           name   = utils::genSha1(in);
    std::filesystem::path fspath = std::filesystem::temp_directory_path() / name;
    std::string           path   = fspath.native();
    auto*                 file   = std::fopen(path.c_str(), "w+");
    {
        va_start(args, fmt);
        std::vfprintf(file, fmt, args);
        va_end(args);
    }
    std::fprintf(file, "\n");
    std::fclose(file);
    return path;
}
