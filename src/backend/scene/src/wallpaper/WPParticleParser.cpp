#include "WPParticleParser.h" 
#include "Particle/ParticleModify.h"
#include <random>
#include <memory>
#include <algorithm>
#include <cmath>
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

std::vector<float> GenRandomVec3(const RandomFn& rf, cFloats min, cFloats max) {
	Floats result {0, 1, 2};
	std::transform(result.begin(), result.end(), result.begin(), [&](float i) {
		return GetRandomIn(min.at(i), max.at(i), rf());
	});
	return result;
}
std::vector<float> GetValidVec(const std::vector<float>& v, uint32_t num, float value) {
	std::vector<float> re(v);
	if(re.size() < num) re.resize(num, value);
	return re;
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
			mapVertex(GetValidVec(ini.min, 3, 0.0f), [](float x) {return x/255.0f;}), 
			mapVertex(GetValidVec(ini.max,3, 0.0f), [](float x) {return x/255.0f;}));
	} else if(ini.name == "lifetimerandom") {
		return [ini, rf](Particle& p) { PM::InitLifetime(p, GetRandomIn(ini.min[0], ini.max[0], rf()));};
	} else if(ini.name == "sizerandom") {
		return [ini, rf](Particle& p) { PM::InitSize(p, GetRandomIn(ini.min[0], ini.max[0], rf())); };
	} else if(ini.name == "alpharandom") {
		return [ini, rf](Particle& p) { PM::InitAlpha(p, GetRandomIn(ini.min[0], ini.max[0], rf())); };
	} else if(ini.name == "velocityrandom") {
		return [ini, rf](Particle& p) { 
			auto result = GenRandomVec3(rf, GetValidVec(ini.min, 3, 0.0f), GetValidVec(ini.max, 3, 0.0f));
			PM::InitVelocity(p, result[0], result[1], result[2]);
		};
	} else if(ini.name == "rotationrandom") {
		return [ini, rf](Particle& p) { 
			auto result = GenRandomVec3(rf, GetValidVec(ini.min, 3, 0.0f), GetValidVec(ini.max, 3, 0.0f));
			PM::InitRotation(p, result[0], result[1], result[2]);
		};
	} else if(ini.name == "angularvelocityrandom") {
		return [ini, rf](Particle& p) { 
			auto result = GenRandomVec3(rf, GetValidVec(ini.min, 3, 0.0f), GetValidVec(ini.max, 3, 0.0f));
			PM::InitAngularVelocity(p, result[0], result[1], result[2]);
		};
	}
	return [](Particle&) {};
}

float FadeValueChange(float life, float start, float end, float startValue, float endValue) {
	if(life <= start) return startValue;
	else if(life > end) return endValue;
	else {
		double pass = (life - start) / (end - start);
		return startValue + ((endValue - startValue) * pass);
	}
}

struct ValueChange {
	float starttime {0};
	float endtime {1.0f};
	float startvalue {1.0f};
	float endvalue {0.0f};

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

static const double pi = std::atan(1)*4;
struct FrequencyValue {
	float frequencymin {0.0f};
	float frequencymax {10.0f};
	float scalemin {0.0f};
	float scalemax {1.0f};
	float phasemin {0.0f};
	float phasemax {2*pi};
	struct StorageRandom {
		float frequency {0.0f};
		float phase {0.0f};
	};
	std::vector<StorageRandom> storage;
	static auto ReadFromJson(const nlohmann::json& j) {
		FrequencyValue v;
		GET_JSON_NAME_VALUE(j, "frequencymin", v.frequencymin);
		GET_JSON_NAME_VALUE(j, "frequencymax", v.frequencymax);
		if(v.frequencymax == 0.0f) v.frequencymax = v.frequencymin;
		GET_JSON_NAME_VALUE(j, "scalemin", v.scalemin);
		GET_JSON_NAME_VALUE(j, "scalemax", v.scalemax);
		GET_JSON_NAME_VALUE(j, "phasemin", v.phasemin);
		GET_JSON_NAME_VALUE(j, "phasemax", v.phasemax);
		return v;
	};
	static void CheckAndResize(FrequencyValue& fv, uint32_t index) {
		if(fv.storage.size() <= index)
			fv.storage.resize(2*(index+1));
	}
	static void GenFrequency(FrequencyValue& fv, Particle& p, uint32_t index, RandomFn& rf) {
		if(!PM::LifetimeOk(p)) fv.storage.at(index).frequency = 0.0f; 
		if(fv.storage.at(index).frequency == 0.0f) {
			fv.storage[index].frequency = GetRandomIn(fv.frequencymin, fv.frequencymax, rf());
			fv.storage[index].phase = GetRandomIn(fv.phasemin, fv.phasemax, rf());
		}
	}
	static double GetScale(const FrequencyValue& fv, uint32_t index, double timePass) {
		auto t = 1.0f / fv.storage.at(index).frequency;
		auto phase = fv.storage.at(index).phase;
		auto value = std::sin(timePass * 2.0f * pi * t + phase) + 1.0f;
		return value*(fv.scalemax-fv.scalemin)*0.5f + fv.scalemin;
	}
};

ParticleOperatorOp WPParticleParser::genParticleOperatorOp(const nlohmann::json& wpj, RandomFn rf) {
	do {
		if(!wpj.contains("name")) break;
		std::string name;
    	GET_JSON_NAME_VALUE(wpj, "name", name);
		if(name == "movement") {
			float drag {0.0f};
			Floats gravity {0.0f};
    		GET_JSON_NAME_VALUE(wpj, "drag", drag);
    		GET_JSON_NAME_VALUE(wpj, "gravity", gravity);
			if(gravity.size() < 3) gravity.resize(3, gravity.at(0));
			return [=](Particle& p, uint32_t, float life, float t){ 
				PM::Accelerate(p, gravity, t);
				PM::MoveByTime(p, t);
			};
		} else if(name == "angularmovement") {
			float drag {0.0f};
			Floats force {0.0f};
    		GET_JSON_NAME_VALUE(wpj, "drag", drag);
    		GET_JSON_NAME_VALUE(wpj, "force", force);
			if(force.size() < 3) force.resize(3, force.at(0));
			return [=](Particle& p, uint32_t, float life, float t){ 
				PM::AngularAccelerate(p, force, t);
				PM::RotateByTime(p, t);
			};
		} else if(name == "alphafade") {
			float fadeintime {0.5f}, fadeouttime {0.5f};
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
			return [vc](Particle& p, uint32_t, float life, float t) {
				PM::MutiplyAlpha(p, FadeValueChange(life, vc));
			};
		} else if(name == "oscillatealpha") {
			FrequencyValue fv = FrequencyValue::ReadFromJson(wpj);
			return [fv,rf](Particle& p, uint32_t index, float life, float t) mutable {
				FrequencyValue::CheckAndResize(fv, index);
				FrequencyValue::GenFrequency(fv, p, index, rf);
				PM::MutiplyAlpha(p, FrequencyValue::GetScale(fv, index, PM::LifetimePassed(p)));
			};
		} else if(name == "oscillatesize") {
			FrequencyValue fv = FrequencyValue::ReadFromJson(wpj);
			return [fv,rf](Particle& p, uint32_t index, float life, float t) mutable {
				FrequencyValue::CheckAndResize(fv, index);
				FrequencyValue::GenFrequency(fv, p, index, rf);
				PM::MutiplySize(p, FrequencyValue::GetScale(fv, index, PM::LifetimePassed(p)));
			};
		} else if(name == "oscillateposition") {
			struct LastMove { double p[3]; };
			std::vector<LastMove> lastMove;
			FrequencyValue fvx = FrequencyValue::ReadFromJson(wpj);
			std::vector<FrequencyValue> fxp = { fvx, fvx, fvx };
			return [=](Particle& p, uint32_t index, float life, float t) mutable {
				std::vector<double> pos(3);
				std::transform(fxp.begin(), fxp.end(), pos.begin(), [&](FrequencyValue& fv) {
					FrequencyValue::CheckAndResize(fv, index);
					FrequencyValue::GenFrequency(fv, p, index, rf);
					return FrequencyValue::GetScale(fv, index, PM::LifetimePassed(p)) * 2.0f;
				});
				if(lastMove.size() <= index) lastMove.resize(2*(index+1), {0.0f});
				double* lastP = lastMove.at(index).p;
				PM::Move(p, pos[0] - lastP[0], pos[1] - lastP[1], 0);
				std::memcpy(lastP, &pos[0], 3*sizeof(double));
			};
		}
	} while(false);
	return [](Particle&, uint32_t, float, float){};
}


ParticleEmittOp WPParticleParser::genParticleEmittOp(const wpscene::Emitter& wpe, RandomFn rf) {
	if(wpe.name == "boxrandom") {
		ParticleBoxEmitterArgs box;
		box.emitSpeed = wpe.rate;
		std::memcpy(box.minDistance, &GetValidVec(wpe.distancemin, 3, wpe.distancemin[0])[0], 3*sizeof(float));
		std::memcpy(box.maxDistance, &GetValidVec(wpe.distancemax, 3, wpe.distancemax[0])[0], 3*sizeof(float));
		box.randomFn = rf;
		std::memcpy(box.directions, &wpe.directions[0], 3*sizeof(float));
		return ParticleBoxEmitterArgs::MakeEmittOp(box);
	} else if(wpe.name == "sphererandom") {
		ParticleSphereEmitterArgs sphere;
		sphere.emitSpeed = wpe.rate;
		sphere.minDistance = wpe.distancemin[0];
		sphere.maxDistance = wpe.distancemax[0];
		std::memcpy(sphere.directions, &wpe.directions[0], 3*sizeof(float));
		sphere.randomFn = rf;
		return ParticleSphereEmitterArgs::MakeEmittOp(sphere);
	} else 
		return [](std::vector<Particle>&, std::vector<ParticleInitOp>&, uint32_t, float) {};
}