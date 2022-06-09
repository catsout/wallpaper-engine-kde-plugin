#pragma once
#include <cstdlib>
#include <string_view>
#include <filesystem>

namespace wallpaper 
{
namespace platform
{

inline std::filesystem::path GetCachePath(std::string_view name) {
    using namespace std::filesystem;

    path p_cache;
    std::string_view home = std::getenv("HOME");
    if(!home.empty()) {
        std::string_view cache = std::getenv("XDG_CACHE_HOME");
        if(cache.empty()) 
            p_cache = path(home) / ".cache";
        else 
            p_cache = path(cache);
    }
    return p_cache / name;
}

}
}
