#include "WPJson.hpp"
#include <nlohmann/json.hpp>

#include "Utils/Identity.hpp"
#include "Utils/String.h"

namespace wallpaper
{

bool ParseJson(const char* file, const char* func, int line, const std::string& source,
               nlohmann::json& result) {
    try {
        result = nlohmann::json::parse(source);
    } catch (nlohmann::json::parse_error& e) {
        WallpaperLog(LOGLEVEL_INFO, func, line, "parse json, %s", e.what());
        return false;
    }
    return true;
}

template<typename T>
inline bool _GetJsonValue(const nlohmann::json&                  json,
                          typename utils::is_std_array<T>::type& value) {
    using Tv          = typename T::value_type;
    const auto* pjson = &json;
    if (json.contains("value")) pjson = &json.at("value");
    const auto& njson = *pjson;
    if (njson.is_number()) {
        value = { njson.get<Tv>() };
        return true;
    } else {
        std::string strvalue;
        strvalue = njson.get<std::string>();
        return utils::StrToArray::Convert(strvalue, value);
    }
}

template<typename T>
inline bool _GetJsonValue(const nlohmann::json& json, T& value) {
    if (json.contains("value"))
        value = json.at("value").get<T>();
    else
        value = json.get<T>();
    return true;
}

template<typename T>
inline bool _GetJsonValue(const char* file, const char* func, int line, const nlohmann::json& json,
                          T& value, bool warn, const char* name) {
    using njson = nlohmann::json;
    std::string nameinfo;
    if (name != nullptr) nameinfo = std::string("(key: ") + name + ")";
    try {
        return _GetJsonValue<T>(json, value);
    } catch (const njson::type_error& e) {
        WallpaperLog(LOGLEVEL_INFO,
                     file,
                     line,
                     "%s %s at %s\n%s",
                     e.what(),
                     nameinfo.c_str(),
                     func,
                     json.dump(4).c_str());
    } catch (const std::invalid_argument& e) {
        WallpaperLog(LOGLEVEL_ERROR, file, line, "%s %s at %s", e.what(), nameinfo.c_str(), func);
    } catch (const std::out_of_range& e) {
        WallpaperLog(LOGLEVEL_ERROR, file, line, "%s %s at %s", e.what(), nameinfo.c_str(), func);
    } catch (const utils::StrToArray::WrongSizeExp& e) {
        WallpaperLog(LOGLEVEL_ERROR, file, line, "%s %s at %s", e.what(), nameinfo.c_str(), func);
    }
    return false;
}

template<typename T>
typename JsonTemplateTypeCheck<T>::type
GetJsonValue(const char* file, const char* func, int line, const nlohmann::json& json, T& value,
             bool has_name, std::string_view name_view, bool warn) {
    std::string name { name_view };
    if(has_name) {
        if (! json.contains(name)) {
            if (warn)
                WallpaperLog(
                    LOGLEVEL_INFO, "", 0, "read json \"%s\" not a key at %s(%s:%d)", name.data(), func, file, line);
            return false;
        } else if (json.at(name).is_null()) {
            if (warn)
                WallpaperLog(
                    LOGLEVEL_INFO, "", 0, "read json \"%s\" is null at %s(%s:%d)", name.data(), func, file, line);
            return false;
        }
    }
    return _GetJsonValue<T>(
        file, func, line, has_name ? json.at(name) : json, value, warn, name.empty() ? nullptr : name.c_str());
}

#define T_IMPL_GET_JSON(TYPE)                                                            \
    template JsonTemplateTypeCheck<TYPE>::type GetJsonValue<TYPE>(const char*,           \
                                                                  const char*,           \
                                                                  int,                   \
                                                                  const nlohmann::json&, \
                                                                  TYPE&,                 \
                                                                  bool,                  \
                                                                  std::string_view,      \
                                                                  bool);

T_IMPL_GET_JSON(bool);
T_IMPL_GET_JSON(int32_t);
T_IMPL_GET_JSON(uint32_t);
T_IMPL_GET_JSON(float);
T_IMPL_GET_JSON(double);
T_IMPL_GET_JSON(std::string);
T_IMPL_GET_JSON(std::vector<float>);

template<std::size_t N>
using iarray = std::array<int, N>;
T_IMPL_GET_JSON(iarray<3>);

template<std::size_t N>
using farray = std::array<float, N>;
T_IMPL_GET_JSON(farray<2>);
T_IMPL_GET_JSON(farray<3>);

// template bool GetJsonValue();
} // namespace wallpaper
