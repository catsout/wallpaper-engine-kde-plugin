#include "WPMaterial.h"

using namespace wallpaper::wpscene;

bool WPMaterialPassBindItem::FromJson(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE(json, "name", name);
    GET_JSON_NAME_VALUE(json, "index", index);
    return true;
}


void WPMaterialPass::Update(const WPMaterialPass& p) {
    int32_t i = -1;
    for(const auto& el:p.textures) {
        i++;
        if(p.textures.size() > textures.size())
            textures.resize(p.textures.size());
        if(!el.empty()) {
            textures[i] = el;
        }
    }
    for(const auto& el:p.constantshadervalues) {
        constantshadervalues[el.first] = el.second;
    }
    for(const auto& el:p.combos) {
        combos[el.first] = el.second;
    }
}

void WPMaterial::MergePass(const WPMaterialPass& p) {
    int32_t i = -1;
    for(const auto& el:p.textures) {
        i++;
        if(p.textures.size() > textures.size())
            textures.resize(p.textures.size());
        if(!el.empty()) {
            textures[i] = el;
        }
    }
    for(const auto& el:p.constantshadervalues) {
        constantshadervalues[el.first] = el.second;
    }
    for(const auto& el:p.combos) {
        combos[el.first] = el.second;
    }
}

bool WPMaterialPass::FromJson(const nlohmann::json& json) {
    if(json.contains("textures")) {
        for(const auto& jT:json.at("textures")) {
            std::string tex;
            if(!jT.is_null())
                GET_JSON_VALUE(jT, tex);
            textures.push_back(tex);
        }
    }
    if(json.contains("constantshadervalues")) {
        for(const auto& jC:json.at("constantshadervalues").items()) {
            std::string name;
            std::vector<float> value;
            GET_JSON_VALUE(jC.key(), name);
            GET_JSON_VALUE(jC.value(), value);
            constantshadervalues[name] = value;
        }
    }
    if(json.contains("combos")) {
        for(const auto& jC:json.at("combos").items()) {
            std::string name;
            int32_t value;
            GET_JSON_VALUE(jC.key(), name);
            GET_JSON_VALUE(jC.value(), value);
            combos[name] = value;
        }
    }
    GET_JSON_NAME_VALUE_NOWARN(json, "target", target);
    if(json.contains("bind")) {
        for(const auto& jB:json.at("bind")) {
            WPMaterialPassBindItem bindItem;
            bindItem.FromJson(jB);
            bind.push_back(bindItem);
        }
    }
    return true;
}

bool WPMaterial::FromJson(const nlohmann::json& json) {
    if(!json.contains("passes") || json.at("passes").size() == 0) {
        LOG_ERROR("material no data");
        return false;
    }
    const auto jContent = json.at("passes").at(0);
    if(!jContent.contains("shader")) {
        LOG_ERROR("material no shader");
        return false;
    }
	GET_JSON_NAME_VALUE(jContent, "blending", blending);
	GET_JSON_NAME_VALUE(jContent, "cullmode", cullmode);
	GET_JSON_NAME_VALUE(jContent, "depthtest", depthtest);
	GET_JSON_NAME_VALUE(jContent, "depthwrite", depthwrite);
	GET_JSON_NAME_VALUE(jContent, "shader", shader);
    if(jContent.contains("textures")) {
        for(const auto& jT:jContent.at("textures")) {
            std::string tex;
            if(!jT.is_null())
                GET_JSON_VALUE(jT, tex);
            textures.push_back(tex);
        }
    }
    if(jContent.contains("constantshadervalues")) {
        for(const auto& jC:jContent.at("constantshadervalues").items()) {
            std::string name;
            std::vector<float> value;
            GET_JSON_VALUE(jC.key(), name);
            GET_JSON_VALUE(jC.value(), value);
            constantshadervalues[name] = value;
        }
    }
    if(jContent.contains("combos")) {
        for(const auto& jC:jContent.at("combos").items()) {
            std::string name;
            int32_t value;
            GET_JSON_VALUE(jC.key(), name);
            GET_JSON_VALUE(jC.value(), value);
            combos[name] = value;
        }
    }
    return true;
}