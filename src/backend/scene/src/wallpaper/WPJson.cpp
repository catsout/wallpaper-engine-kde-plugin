#include "WPJson.h"

bool wallpaper::ParseJson(const char* func, int line, const std::string& source, nlohmann::json& result) {
    try {
        result = nlohmann::json::parse(source);
    } catch(nlohmann::json::parse_error& e) {
        Logger::Log(func, line, "Error parse json ", e.what());
        return false;
    }
    return true;
}


