#include "WPJson.h"

bool wallpaper::ParseJson(const char* func, int line, const std::string& source, nlohmann::json& result) {
    int x;
    try {
        result = nlohmann::json::parse(source);
    } catch(nlohmann::json::parse_error& e) {
		WallpaperLog(LOGLEVEL_INFO, func, line, "parse json, %s", e.what());
        return false;
    }
    return true;
}