#include "WPParticleParser.h" 
#include "Particle/ParticleModify.h"
#include <random>
#include <memory>
#include <algorithm>
#include "Log.h"

using namespace wallpaper;
using PM = ParticleModify;

static std::uniform_real_distribution<float> ur(0.0f, 1.0f);
typedef const std::vector<float>& cFloats;
typedef std::vector<float> Floats;
typedef std::function<float()> RandomFn;

static float GetRandomIn(float min, float max, float random) {
	return min + (max - min)*random;
}

void Color(Particle& p, RandomFn& rf, cFloats min, cFloats max) {
	Floats result {0, 1, 2};
	float random = rf();
	std::transform(result.begin(), result.end(), result.begin(), [&](float i) {
		return GetRandomIn(min.at(i), max.at(i), random);
	});
	PM::InitColor(p, result[0], result[1], result[2]);
}

void Velocity(Particle& p, RandomFn& rf, cFloats min, cFloats max) {
	Floats result {0, 1, 2};
	std::transform(result.begin(), result.end(), result.begin(), [&](float i) {
		return GetRandomIn(min.at(i), max.at(i), rf());
	});
	PM::InitVelocity(p, result[0], result[1], result[2]);
}

ParticleInitOp WPParticleParser::genParticleInitOp(const wpscene::Initializer& ini, RandomFn rf) {
	using namespace std::placeholders;
	auto mapVertex = [](const std::vector<float>& v, float(*oper)(float)) {
		std::vector<float> result; result.reserve(v.size());
		for(const auto& x:v) result.push_back(oper(x));
		return result;
	};
	if(ini.name == "colorrandom") {
		return std::bind(Color, _1, rf, 
			mapVertex(ini.min, [](float x) {return x/255.0f;}), 
			mapVertex(ini.max, [](float x) {return x/255.0f;}));
	} else if(ini.name == "lifetimerandom") {
		return [ini, rf](Particle& p) { PM::InitLifetime(p, GetRandomIn(ini.min[0], ini.max[0], rf()));};
	} else if(ini.name == "sizerandom") {
		return [ini, rf](Particle& p) { PM::InitSize(p, GetRandomIn(ini.min[0], ini.max[0], rf())); };
	} else if(ini.name == "alpharandom") {
		return [ini, rf](Particle& p) { PM::InitAlpha(p, GetRandomIn(ini.min[0], ini.max[0], rf())); };
	} else if(ini.name == "velocityrandom") {
		return std::bind(Velocity, _1, rf, ini.min, ini.max);
	}


	return [](Particle&) {};
}

float FadeValueChange(float life, float start, float end, float startValue, float endValue) {
	if(life <= start) return startValue;
	else if(life > end) return endValue;
	else {
		float pass = (life - start) / (end - start);
		return startValue + ((endValue - startValue) * pass);
	}
}

struct ValueChange {
	float starttime {0};
	float endtime {1.0f};
	float startvalue {0};
	float endvalue {1.0f};

	static auto ReadFromJson(const nlohmann::json& j) {
		ValueChange v;
		GET_JSON_NAME_VALUE(j, "starttime", v.starttime);
		GET_JSON_NAME_VALUE(j, "endtime", v.endtime);
		GET_JSON_NAME_VALUE(j, "startvalue", v.startvalue);
		GET_JSON_NAME_VALUE(j, "endvalue", v.endvalue);
		return v;
	}
};
float FadeValueChange(float life, const ValueChange& v) {
	if(life <= v.starttime) return v.startvalue;
	else if(life > v.endtime) return v.endvalue;
	else {
		float pass = (life - v.starttime) / (v.endtime - v.starttime);
		return v.startvalue + ((v.endvalue - v.startvalue) * pass);
	}
}

struct FrequencyValue {
	float frequencymin {1.0f};
	float frequencymax {1.0f};
	float scalemin {0.0f};
	float scalemax {0.0f};
	std::vector<float> frequency;
	static auto ReadFromJson(const nlohmann::json& j) {
		FrequencyValue v;
		GET_JSON_NAME_VALUE(j, "frequencymin", v.frequencymin);
		GET_JSON_NAME_VALUE(j, "frequencymax", v.frequencymax);
		GET_JSON_NAME_VALUE(j, "scalemin", v.scalemin);
		GET_JSON_NAME_VALUE(j, "scalemax", v.scalemax);
		return v;
	}
};

ParticleOperatorOp WPParticleParser::genParticleOperatorOp(const nlohmann::json& wpj, RandomFn rf) {
	do {
		if(!wpj.contains("name")) break;
		std::string name;
    	GET_JSON_NAME_VALUE(wpj, "name", name);
		if(name == "movement") {
			float drag {0};
			Floats gravity;
    		GET_JSON_NAME_VALUE(wpj, "drag", drag);
    		GET_JSON_NAME_VALUE(wpj, "gravity", gravity);
			if(gravity.size() == 1) gravity.resize(3, gravity.at(0));
			else if(gravity.size() != 3) {
				LOG_ERROR("Read gravity error");
			}
			return [](Particle& p, uint32_t, float life, float t){ 
				PM::MoveByTime(p, t);
			};
		} else if(name == "alphafade") {
			float fadeintime {0}, fadeouttime {0};
    		GET_JSON_NAME_VALUE(wpj, "fadeintime", fadeintime);
    		GET_JSON_NAME_VALUE(wpj, "fadeouttime", fadeouttime);
			return [fadeintime,fadeouttime](Particle& p, uint32_t, float life, float t){
				if(life <= fadeintime) PM::MutiplyAlpha(p, FadeValueChange(life, 0, fadeintime, 0, 1.0f));
				else if(life > fadeouttime) PM::MutiplyAlpha(p, FadeValueChange(life, fadeouttime, 1.0f, 1.0f, 0));
			};
		} else if(name == "sizechange") {
			auto vc = ValueChange::ReadFromJson(wpj);
			return [vc](Particle& p, uint32_t, float life, float t){
				PM::MutiplySize(p, FadeValueChange(life, vc));
			};
		} else if(name == "alphachange") {
			auto vc = ValueChange::ReadFromJson(wpj);
			return [vc](Particle& p, uint32_t, float life, float t){
				PM::MutiplyAlpha(p, FadeValueChange(life, vc));
			};
		} else if(name == "oscillatealpha") {
			FrequencyValue fv = FrequencyValue::ReadFromJson(wpj);

		}
	} while(false);
	return [](Particle&, uint32_t, float, float){};
}