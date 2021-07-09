#include "WPParticleObject.h"

#include "pkg.h"
#include "wallpaper.h"

#include "Log.h"

using namespace wallpaper::wpscene;

bool Emitter::FromJson(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE(json, "name", name);
    GET_JSON_NAME_VALUE(json, "id", id);
    GET_JSON_NAME_VALUE_NOWARN(json, "distancemax", distancemax);
    GET_JSON_NAME_VALUE_NOWARN(json, "distancemin", distancemin);
    GET_JSON_NAME_VALUE_NOWARN(json, "rate", rate);
    GET_JSON_NAME_VALUE_NOWARN(json, "directions", directions);
    GET_JSON_NAME_VALUE_NOWARN(json, "origin", origin);
    return true;
}

bool ParticleInstanceoverride::FromJosn(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE_NOWARN(json, "alpha", alpha);
    GET_JSON_NAME_VALUE_NOWARN(json, "size", size);
    GET_JSON_NAME_VALUE_NOWARN(json, "lifetime", lifetime);
    GET_JSON_NAME_VALUE_NOWARN(json, "rate", rate);
    GET_JSON_NAME_VALUE_NOWARN(json, "speed", speed);
    GET_JSON_NAME_VALUE_NOWARN(json, "count", count);
    GET_JSON_NAME_VALUE_NOWARN(json, "colorn", colorn);
    if(colorn.size() == 1) colorn.resize(3, colorn[0]);
    return true;
};

bool Particle::FromJson(const nlohmann::json& json) {
    if(!json.contains("emitter")) return false;
    for(const auto& el:json.at("emitter")) {
        Emitter emi;
        emi.FromJson(el);
        emitters.push_back(emi);
    }
    if(!json.contains("initializer")) return false;
    if(!json.contains("operator")) return false;

    for(const auto& el:json.at("initializer")) {
        initializers.push_back(el);
    }
    for(const auto& el:json.at("operator")) {
        operators.push_back(el);
    }
	GET_JSON_NAME_VALUE(json, "maxcount", maxcount);
	GET_JSON_NAME_VALUE(json, "starttime", starttime);
    return true;
}

bool WPParticleObject::FromJson(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE(json, "particle", particle);
    GET_JSON_NAME_VALUE_NOWARN(json, "visible", visible);

	GET_JSON_NAME_VALUE_NOWARN(json, "name", name);
	GET_JSON_NAME_VALUE_NOWARN(json, "id", id);
    LOG_INFO(name);
	GET_JSON_NAME_VALUE(json, "origin", origin);	
	GET_JSON_NAME_VALUE(json, "angles", angles);	
	GET_JSON_NAME_VALUE(json, "scale", scale);	
	GET_JSON_NAME_VALUE_NOWARN(json, "parallaxDepth", parallaxDepth);

    if(json.contains("instanceoverride") && !json.at("instanceoverride").is_null()) {
        instanceoverride.FromJosn(json.at("instanceoverride"));
    }

    nlohmann::json jParticle;
    if(!PARSE_JSON(fs::GetContent(WallpaperGL::GetPkgfs(), particle), jParticle))
        return false;
    if(!particleObj.FromJson(jParticle))
        return false;
    if(jParticle.contains("material")) {
        std::string matPath;
		GET_JSON_NAME_VALUE(jParticle, "material", matPath);	
        nlohmann::json jMat;
        if(!PARSE_JSON(fs::GetContent(WallpaperGL::GetPkgfs(), matPath), jMat))
            return false;
        material.FromJson(jMat);
    } else {
        LOG_INFO("image object no material");
        return false;
    }
    return true;
}