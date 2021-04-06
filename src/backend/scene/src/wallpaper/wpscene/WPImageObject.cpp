#include "WPImageObject.h"
#include "pkg.h"
#include "wallpaper.h"
#include "common.h"

using namespace wallpaper::wpscene;

bool WPEffectFbo::FromJson(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE(json, "name", name);
    GET_JSON_NAME_VALUE(json, "format", format);
    GET_JSON_NAME_VALUE(json, "scale", scale);
    return true;
}

bool WPImageEffect::FromJson(const nlohmann::json& json) {
    std::string filePath;
    GET_JSON_NAME_VALUE(json, "file", filePath);
    GET_JSON_NAME_VALUE_NOWARN(json, "visible", visible);
    GET_JSON_NAME_VALUE_NOWARN(json, "name", name);
	GET_JSON_NAME_VALUE_NOWARN(json, "id", id);
    const auto jEffect = nlohmann::json::parse(fs::GetContent(WallpaperGL::GetPkgfs(), filePath));
	GET_JSON_NAME_VALUE_NOWARN(jEffect, "version", version);
    if(name.empty()) {
        GET_JSON_NAME_VALUE(jEffect, "name", name);
    }
    if(jEffect.contains("fbos")) {
        for(auto& jF:jEffect.at("fbos")) {
            WPEffectFbo fbo;
            fbo.FromJson(jF);
            fbos.push_back(std::move(fbo));
        }
    }
    if(jEffect.contains("passes")) {
        const auto& jEPasses = jEffect.at("passes");
        bool compose {false};
        for(const auto& jP:jEPasses) {
            if(!jP.contains("material")) {
                if(jP.contains("command"))
                    continue;
                LOG_ERROR("no material in effect pass");
                return false;
            }
            std::string matPath;
            GET_JSON_NAME_VALUE(jP, "material", matPath);
            const auto jMat = nlohmann::json::parse(fs::GetContent(WallpaperGL::GetPkgfs(), matPath));
            WPMaterial material;
            material.FromJson(jMat);
            materials.push_back(std::move(material));
            WPMaterialPass pass;
            pass.FromJson(jP);
            passes.push_back(std::move(pass));
            if(jP.contains("compose"))
	            GET_JSON_NAME_VALUE(jP, "compose", compose);
        }
        if(compose) {
            if(passes.size() != 2) {
                LOG_ERROR("effect compose option error");
            }
        }
    } else {
        LOG_ERROR("no passes in effect file");
        return false;
    }
    if(json.contains("passes")) {
        const auto& jPasses = json.at("passes");
        if(jPasses.size() > passes.size()) {
            LOG_ERROR("passes is not injective");
            return false;
        }
        int32_t i = 0;
        for(const auto& jP:jPasses) {
            WPMaterialPass pass;
            pass.FromJson(jP);
            passes[i++].Update(pass); 
        }
    }
    return true;
}

bool WPImageObject::FromJson(const nlohmann::json& json) {
    std::string imagePath;
    GET_JSON_NAME_VALUE(json, "image", imagePath);
    GET_JSON_NAME_VALUE_NOWARN(json, "visible", visible);
    auto image = nlohmann::json::parse(fs::GetContent(WallpaperGL::GetPkgfs(), imagePath));
    GET_JSON_NAME_VALUE_NOWARN(image, "fullscreen", fullscreen);
	GET_JSON_NAME_VALUE_NOWARN(json, "name", name);
	GET_JSON_NAME_VALUE_NOWARN(json, "id", id);
    LOG_INFO(name);
	if(!fullscreen) {
		GET_JSON_NAME_VALUE(json, "origin", origin);	
		GET_JSON_NAME_VALUE(json, "angles", angles);	
		GET_JSON_NAME_VALUE(json, "scale", scale);	
		GET_JSON_NAME_VALUE_NOWARN(json, "parallaxDepth", parallaxDepth);
		if(image.contains("width")) {
			int32_t w,h;
			GET_JSON_NAME_VALUE(image, "width", w);	
			GET_JSON_NAME_VALUE(image, "height", h);	
			size = {(float)w, (float)h};
		} else {
			GET_JSON_NAME_VALUE(json, "size", size);	
		}  
    }
    GET_JSON_NAME_VALUE_NOWARN(json, "color", color);
    GET_JSON_NAME_VALUE_NOWARN(json, "alpha", alpha);
    GET_JSON_NAME_VALUE_NOWARN(json, "brightness", brightness);
    if(image.contains("material")) {
        std::string matPath;
		GET_JSON_NAME_VALUE(image, "material", matPath);	
        auto jMat = nlohmann::json::parse(fs::GetContent(WallpaperGL::GetPkgfs(), matPath));
        material.FromJson(jMat);
    } else {
        LOG_INFO("image object no material");
        return false;
    }
    if(json.contains("effects")) {
        for(const auto& jE:json.at("effects")) {
            WPImageEffect wpeff;
            wpeff.FromJson(jE);
            effects.push_back(std::move(wpeff));
        }
    }
    return true;
}