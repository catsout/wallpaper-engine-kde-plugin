#include "WPJson.hpp"

bool wallpaper::ParseJson(const char* file, const char* func, int line, const std::string& source,
                          nlohmann::json& result) {
    try {
        result = nlohmann::json::parse(source);
    } catch (nlohmann::json::parse_error& e) {
        WallpaperLog(LOGLEVEL_INFO, func, line, "parse json, %s", e.what());
        return false;
    }
    return true;
}