#pragma once
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string_view>
#include <type_traits>

#include "Utils/Logging.h"

#define GET_JSON_VALUE(json, value) \
    wallpaper::GetJsonValue(        \
        __SHORT_FILE__, __FUNCTION__, __LINE__, (json), (value), false, nullptr, false)
#define GET_JSON_NAME_VALUE(json, name, value) \
    wallpaper::GetJsonValue(                   \
        __SHORT_FILE__, __FUNCTION__, __LINE__, (json), (value), true, (name), false)

#define GET_JSON_VALUE_NOWARN(json, value) \
    wallpaper::GetJsonValue(               \
        __SHORT_FILE__, __FUNCTION__, __LINE__, (json), (value), false, nullptr, true)
#define GET_JSON_NAME_VALUE_NOWARN(json, name, value) \
    wallpaper::GetJsonValue(                          \
        __SHORT_FILE__, __FUNCTION__, __LINE__, (json), (value), true, (name), true)

#define PARSE_JSON(source, result) \
    wallpaper::ParseJson(__SHORT_FILE__, __FUNCTION__, __LINE__, (source), (result))

namespace wallpaper
{

template<typename T>
struct JsonTemplateTypeCheck {
    using type = bool;
    static_assert(! std::is_const_v<T>, "GetJsonValue need a non const value");
};

template<typename T>
typename wallpaper::JsonTemplateTypeCheck<T>::type
GetJsonValue(const char* file, const char* func, int line, const nlohmann::json& json, T& value,
             bool has_name, std::string_view name, bool warn);

bool ParseJson(const char* file, const char* func, int line, const std::string& source,
               nlohmann::json& result);
} // namespace wallpaper
