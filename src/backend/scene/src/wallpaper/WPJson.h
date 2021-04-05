#pragma once
#include <nlohmann/json.hpp>
#include "common.h"

#define GET_JSON_VALUE(json, value) wallpaper::GetJsonValue(__FUNCTION__, __LINE__, (json), (value))
#define GET_JSON_NAME_VALUE(json, name, value) wallpaper::GetJsonValue(__FUNCTION__, __LINE__, (json), (name), (value))
#define GET_JSON_VALUE_NOWARN(json, value) wallpaper::GetJsonValue(__FUNCTION__, __LINE__, (json), (value), false)
#define GET_JSON_NAME_VALUE_NOWARN(json, name, value) wallpaper::GetJsonValue(__FUNCTION__, __LINE__, (json), (name), (value), false)

namespace wallpaper {
	template <typename T>
	bool GetJsonValue(const nlohmann::json& json, T& value) {
		if(json.contains("value") && json.contains("user")) 
			value = json.at("value").get<T>();
		else value = json.get<T>();
		return true;
	}

	template<> inline
	bool wallpaper::GetJsonValue<std::vector<float>>(const nlohmann::json& json, std::vector<float>& value) {
		const auto* pjson = &json;
		if(json.contains("value")) 
			pjson = &json.at("value");
		const auto& njson = *pjson;
		if(njson.is_number()) {
			value = {njson.get<float>()};
			return true;
		}
		else {
			std::string strvalue;
			strvalue = njson.get<std::string>();
			return StringToVec<float>(strvalue, value);
		}
	}

	template <typename T>
	bool GetJsonValue(const char* func, int line, const nlohmann::json& json, T& value, const char* name = nullptr) {
		using njson = nlohmann::json;
		std::string nameinfo;
		if(name != nullptr) 
			nameinfo = std::string("(key: ") + name + ")";
		try {
			return GetJsonValue<T>(json, value);
		} catch(njson::type_error& e) {
			Logger::Log(func, line, "Error read json at: ", e.what() + nameinfo);
		}
		return false; 
	}

	template <typename T>
	bool GetJsonValue(const char* func, int line, const nlohmann::json& json, const std::string& name, T& value, bool warn = true) {
		if(!json.contains(name)) {
			if(warn)
				Logger::Log(func, line, "Warning read json: ", name + " not a key");
			return false;
		}
		return GetJsonValue<T>(func, line, json.at(name), value, name.c_str());
	}
}
