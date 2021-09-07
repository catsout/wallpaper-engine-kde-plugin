#pragma once
#include <nlohmann/json.hpp>
#include <cstdint>
#include "Utils/String.h"
#include "Utils/Logging.h"

#define GET_JSON_VALUE(json, value) wallpaper::GetJsonValue(__SHORT_FILE__,__FUNCTION__, __LINE__, (json), (value))
#define GET_JSON_NAME_VALUE(json, name, value) wallpaper::GetJsonValue(__SHORT_FILE__,__FUNCTION__, __LINE__, (json), (name), (value))
#define GET_JSON_VALUE_NOWARN(json, value) wallpaper::GetJsonValue(__SHORT_FILE__,__FUNCTION__, __LINE__, (json), (value), false)
#define GET_JSON_NAME_VALUE_NOWARN(json, name, value) wallpaper::GetJsonValue(__SHORT_FILE__,__FUNCTION__, __LINE__, (json), (name), (value), false)
#define PARSE_JSON(source, result) wallpaper::ParseJson(__SHORT_FILE__,__FUNCTION__, __LINE__, (source), (result))

namespace wallpaper {

	template <class T>
	struct is_std_vector { static const bool value=false; };

	template <class T>
	struct is_std_vector<std::vector<T>> { static const bool value=true; };

	template <typename T>
	bool GetJsonValue(const nlohmann::json& json, typename std::enable_if<is_std_vector<T>::value, T>::type& value) {
		using Tv = typename T::value_type;
		const auto* pjson = &json;
		if(json.contains("value")) 
			pjson = &json.at("value");
		const auto& njson = *pjson;
		if(njson.is_number()) {
			value = {njson.get<Tv>()};
			return true;
		}
		else {
			std::string strvalue;
			strvalue = njson.get<std::string>();
			return utils::StringToVec<Tv>(strvalue, value);
		}
	}

	template <typename T>
	bool GetJsonValue(const nlohmann::json& json, typename std::enable_if<!is_std_vector<T>::value, T>::type& value) {;
		if(json.contains("value")) 
			value = json.at("value").get<T>();
		else value = json.get<T>();
		return true;
	}

	template <typename T>
	bool GetJsonValue(const char* file, const char* func, int line, const nlohmann::json& json, T& value, const char* name = nullptr) {
		using njson = nlohmann::json;
		std::string nameinfo;
		if(name != nullptr) 
			nameinfo = std::string("(key: ") + name + ")";
		try {
			return GetJsonValue<T>(json, value);
		} catch(njson::type_error& e) {
			WallpaperLog(LOGLEVEL_INFO, file, line, "%s %s at %s\n%s", e.what(), nameinfo.c_str(), func, json.dump(4).c_str());
		}
		return false; 
	}

	template <typename T>
	bool GetJsonValue(const char* file,const char* func, int line, const nlohmann::json& json, const std::string& name, T& value, bool warn = true) {
		if(!json.contains(name)) {
			if(warn)
				WallpaperLog(LOGLEVEL_INFO, file, line, "read json \"%s\" not a key at %s", name.data(), func);
			return false;
		} else if(json.at(name).is_null()) {
			if(warn)
				WallpaperLog(LOGLEVEL_INFO, func, line, "read json \"%s\" is null at %s", name.data(), func);
			return false;
		}
		return GetJsonValue<T>(file, func, line, json.at(name), value, name.c_str());
	}

	bool ParseJson(const char* file,const char* func, int line, const std::string& source, nlohmann::json& result);
}
