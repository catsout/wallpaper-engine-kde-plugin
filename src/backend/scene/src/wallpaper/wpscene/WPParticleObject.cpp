#include "WPParticleObject.h"

#include "pkg.h"
#include "wallpaper.h"

#include "Log.h"

using namespace wallpaper::wpscene;

bool ParticleRender::FromJson(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE(json, "name", name);
    if(name == "spritetrail") {
        GET_JSON_NAME_VALUE_NOWARN(json, "length", length);
        GET_JSON_NAME_VALUE_NOWARN(json, "maxlengh", maxlength);
    }
    return true;
}

bool Emitter::FromJson(const nlohmann::json& json) {
    GET_JSON_NAME_VALUE(json, "name", name);
    GET_JSON_NAME_VALUE(json, "id", id);
    GET_JSON_NAME_VALUE_NOWARN(json, "distancemax", distancemax);
    GET_JSON_NAME_VALUE_NOWARN(json, "distancemin", distancemin);
    GET_JSON_NAME_VALUE_NOWARN(json, "rate", rate);
    GET_JSON_NAME_VALUE_NOWARN(json, "directions", directions);
    GET_JSON_NAME_VALUE_NOWARN(json, "origin", origin);
    GET_JSON_NAME_VALUE_NOWARN(json, "sign", sign);
    std::transform(sign.begin(), sign.end(), sign.begin(), [](int32_t v) {
        if(v != 0)
            return v / std::abs(v);
        else return 0;
    });
    return true;
}

bool ParticleInstanceoverride::FromJosn(const nlohmann::json& json) {
    enabled = true;
    GET_JSON_NAME_VALUE_NOWARN(json, "alpha", alpha);
    GET_JSON_NAME_VALUE_NOWARN(json, "size", size);
    GET_JSON_NAME_VALUE_NOWARN(json, "lifetime", lifetime);
    GET_JSON_NAME_VALUE_NOWARN(json, "rate", rate);
    GET_JSON_NAME_VALUE_NOWARN(json, "speed", speed);
    GET_JSON_NAME_VALUE_NOWARN(json, "count", count);
    if(json.contains("color")) {
        GET_JSON_NAME_VALUE(json, "color", color);
        if(color.size() == 1) color.resize(3, color[0]);
        overColor = true;
    } else if(json.contains("colorn")) {
        GET_JSON_NAME_VALUE(json, "colorn", colorn);
        if(colorn.size() == 1) colorn.resize(3, colorn[0]);
        overColorn = true;
    }
    return true;
};

bool Particle::FromJson(const nlohmann::json& json) {
    if(!json.contains("emitter")) {
        LOG_ERROR("particle no emitter");
        return false;
    }
    for(const auto& el:json.at("emitter")) {
        Emitter emi;
        emi.FromJson(el);
        emitters.push_back(emi);
    }
    if(json.contains("renderer"))  {
        for(const auto& el:json.at("renderer")) {
            ParticleRender pr;
            pr.FromJson(el);
            renderers.push_back(pr);
        }
    }
    if(json.contains("initializer")) {
        for(const auto& el:json.at("initializer")) {
            initializers.push_back(el);
        }
    }
    if(json.contains("operator")) {
        for(const auto& el:json.at("operator")) {
            operators.push_back(el);
        }
    }
	GET_JSON_NAME_VALUE(json, "animationmode", animationmode);
	GET_JSON_NAME_VALUE(json, "maxcount", maxcount);
	GET_JSON_NAME_VALUE(json, "starttime", starttime);

    int32_t rawflags {0};
	GET_JSON_NAME_VALUE_NOWARN(json, "flags", rawflags);
    if(rawflags > 0) {
        flags.wordspace = rawflags & 1;
        flags.spritenoframeblending = rawflags & 2;
        flags.perspective = rawflags & 4;
    }
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
        LOG_ERROR("particle object no material");
        return false;
    }
    return true;
}