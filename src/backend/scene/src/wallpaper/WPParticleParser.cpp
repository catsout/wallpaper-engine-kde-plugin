#include "WPParticleParser.h" 
#include "Particle/ParticleModify.h"
#include <random>
#include <memory>
#include <algorithm>
#include <cmath>

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "Utils/Log.h"

using namespace wallpaper;
using namespace Eigen;
using PM = ParticleModify;

static std::uniform_real_distribution<float> ur(0.0f, 1.0f);
typedef const std::vector<float>& cFloats;
typedef std::vector<float> Floats;
typedef std::function<float()> RandomFn;

static float GetRandomIn(float min, float max, float random) {
	return min + (max - min)*random;
}

void Color(Particle& p, RandomFn& rf, cFloats min, cFloats max) {
	float random = rf();
	Vector3f result(3);
	for(int32_t i=0;i<3;i++) {
		result[i] = GetRandomIn(min[i], max[i], random);
	}
	PM::InitColor(p, result[0], result[1], result[2]);
}

Vector3f GenRandomVec3(const RandomFn& rf, cFloats min, cFloats max) {
	Vector3f result(3);
	for(int32_t i=0;i<3;i++) {
		result[i] = GetRandomIn(min[i], max[i], rf());
	}
	return result;
}
std::vector<float> GetValidVec(const std::vector<float>& v, uint32_t num, float value) {
	std::vector<float> re(v);
	if(re.size() < num) re.resize(num, value);
	return re;
}

struct SingleRandom {
	float min {0.0f};
	float max {0.0f};
	float exponent {1.0f};
	static void ReadFromJson(const nlohmann::json& j, SingleRandom& r) {
		GET_JSON_NAME_VALUE_NOWARN(j, "min", r.min);
		GET_JSON_NAME_VALUE_NOWARN(j, "max", r.max);
	};
};
struct VecRandom {
	Floats min {0.0f, 0.0f, 0.0f};
	Floats max {0.0f, 0.0f, 0.0f};
	float exponent {1.0f};
	static void ReadFromJson(const nlohmann::json& j, VecRandom& r) {
		GET_JSON_NAME_VALUE_NOWARN(j, "min", r.min);
		GET_JSON_NAME_VALUE_NOWARN(j, "max", r.max);
		r.min = GetValidVec(r.min, 3, r.min[0]);
		r.max = GetValidVec(r.max, 3, r.max[0]);
	};
};
struct TurbulentRandom{
	float scale {1.0f};
    float timescale {1.0f};
    float offset {0.0f};
    float speedmin {100.0f};
    float speedmax {250.0f};
    float phasemin {0.0f};
    float phasemax {0.1f};
    Floats forward {0.0f, 1.0f, 0.0f}; // x y z
    Floats right {0.0f, 0.0f, 1.0f};
    Floats up {1.0f, 0.0f, 0.0f};
	static void ReadFromJson(const nlohmann::json& j, TurbulentRandom& r) {
		GET_JSON_NAME_VALUE_NOWARN(j, "scale", r.scale);
		GET_JSON_NAME_VALUE_NOWARN(j, "timescale", r.timescale);
		GET_JSON_NAME_VALUE_NOWARN(j, "offset", r.offset);
		GET_JSON_NAME_VALUE_NOWARN(j, "speedmin", r.speedmin);
		GET_JSON_NAME_VALUE_NOWARN(j, "speedmax", r.speedmax);
		GET_JSON_NAME_VALUE_NOWARN(j, "phasemin", r.phasemin);
		GET_JSON_NAME_VALUE_NOWARN(j, "phasemax", r.phasemax);
		GET_JSON_NAME_VALUE_NOWARN(j, "forward", r.forward);
		GET_JSON_NAME_VALUE_NOWARN(j, "right", r.right);
		GET_JSON_NAME_VALUE_NOWARN(j, "up", r.up);
	};
};
Floats mapVertex(cFloats v, float(*oper)(float)) {
	Floats result; result.reserve(v.size());
	for(const auto& x:v) result.push_back(oper(x));
	return result;
};


ParticleInitOp WPParticleParser::genParticleInitOp(const nlohmann::json& wpj, RandomFn rf) {
	using namespace std::placeholders;
	do {
		if(!wpj.contains("name")) break;
		std::string name;
		GET_JSON_NAME_VALUE(wpj, "name", name);

		if(name == "colorrandom") {
			VecRandom r;
			r.min = {0.0f, 0.0f, 0.0f};
			r.max = {255.0f, 255.0f, 255.0f};
			VecRandom::ReadFromJson(wpj, r);
			return std::bind(Color, _1, rf, 
				mapVertex(r.min, [](float x) {return x/255.0f;}), 
				mapVertex(r.max, [](float x) {return x/255.0f;}));
		} else if(name == "lifetimerandom") {
			SingleRandom r = {0.0f, 1.0f};
			SingleRandom::ReadFromJson(wpj, r);
			return [=](Particle& p) { PM::InitLifetime(p, GetRandomIn(r.min, r.max, rf()));};
		} else if(name == "sizerandom") {
			SingleRandom r = {0.0f, 20.0f};
			SingleRandom::ReadFromJson(wpj, r);
			return [=](Particle& p) { PM::InitSize(p, GetRandomIn(r.min, r.max, rf())); };
		} else if(name == "alpharandom") {
			SingleRandom r = {1.0f, 1.0f};
			SingleRandom::ReadFromJson(wpj, r);
			return [=](Particle& p) { PM::InitAlpha(p, GetRandomIn(r.min, r.max, rf())); };
		} else if(name == "velocityrandom") {
			VecRandom r;
			VecRandom::ReadFromJson(wpj, r);
			return [=](Particle& p) { 
				auto result = GenRandomVec3(rf, r.min, r.max);
				PM::ChangeVelocity(p, result[0], result[1], result[2]);
			};
		} else if(name == "rotationrandom") {
			VecRandom r;
			VecRandom::ReadFromJson(wpj, r);
			return [=](Particle& p) { 
				auto result = GenRandomVec3(rf, r.min, r.max);
				PM::ChangeRotation(p, result[0], result[1], result[2]);
			};
		} else if(name == "angularvelocityrandom") {
			VecRandom r;
			VecRandom::ReadFromJson(wpj, r);
			return [=](Particle& p) { 
				auto result = GenRandomVec3(rf, r.min, r.max);
				PM::ChangeAngularVelocity(p, result[0], result[1], result[2]);
			};
		} else if(name  == "turbulentvelocityrandom") {
			// to do
			TurbulentRandom r;
			TurbulentRandom::ReadFromJson(wpj, r);
			return [=](Particle& p) {
				float speed = GetRandomIn(r.speedmin, r.speedmax, rf());
				Vector3f result = Vector3f(&r.forward[0]) * speed;
				result = AngleAxisf(r.offset, Vector3f(&r.right[0]).normalized()) * result;
				return PM::ChangeVelocity(p, result.x(), result.y(), result.z());
			};
		}
	} while(false);
	return [](Particle&) {};
}

ParticleInitOp WPParticleParser::genOverrideInitOp(const wpscene::ParticleInstanceoverride& over) {
	return [=](Particle& p) {
		PM::MutiplyInitLifeTime(p, over.lifetime);
		PM::MutiplyInitAlpha(p, over.alpha);
		PM::MutiplyInitSize(p, over.size);
		PM::MutiplyVelocity(p, over.speed);
		if(over.overColor) {
			PM::InitColor(p, over.color[0]/255.0f, over.color[1]/255.0f, over.color[2]/255.0f);
		} else if(over.overColorn) {
			PM::MutiplyInitColor(p, over.colorn[0], over.colorn[1], over.colorn[2]);
		}
	};
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
		GET_JSON_NAME_VALUE_NOWARN(j, "starttime", v.starttime);
		GET_JSON_NAME_VALUE_NOWARN(j, "endtime", v.endtime);
		GET_JSON_NAME_VALUE_NOWARN(j, "startvalue", v.startvalue);
		GET_JSON_NAME_VALUE_NOWARN(j, "endvalue", v.endvalue);
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

struct VecChange {
	float starttime {0};
	float endtime {1.0f};
	Floats startvalue {0.0f, 0.0f, 0.0f};
	Floats endvalue {0.0f, 0.0f, 0.0f};
	static auto ReadFromJson(const nlohmann::json& j) {
		VecChange v;
		GET_JSON_NAME_VALUE_NOWARN(j, "starttime", v.starttime);
		GET_JSON_NAME_VALUE_NOWARN(j, "endtime", v.endtime);
		GET_JSON_NAME_VALUE_NOWARN(j, "startvalue", v.startvalue);
		GET_JSON_NAME_VALUE_NOWARN(j, "endvalue", v.endvalue);
		return v;
	}
};


static const double pi = std::atan(1)*4;
struct FrequencyValue {
	Floats mask {1.0f, 1.0f, 1.0f};
	float frequencymin {0.0f};
	float frequencymax {10.0f};
	float scalemin {0.0f};
	float scalemax {1.0f};
	float phasemin {0.0f};
	float phasemax {static_cast<float>(2*pi)};
	struct StorageRandom {
		float frequency {0.0f};
		float phase {0.0f};
	};
	std::vector<StorageRandom> storage;
	static auto ReadFromJson(const nlohmann::json& j) {
		FrequencyValue v;
		GET_JSON_NAME_VALUE_NOWARN(j, "frequencymin", v.frequencymin);
		GET_JSON_NAME_VALUE_NOWARN(j, "frequencymax", v.frequencymax);
		if(v.frequencymax == 0.0f) v.frequencymax = v.frequencymin;
		GET_JSON_NAME_VALUE_NOWARN(j, "scalemin", v.scalemin);
		GET_JSON_NAME_VALUE_NOWARN(j, "scalemax", v.scalemax);
		GET_JSON_NAME_VALUE_NOWARN(j, "phasemin", v.phasemin);
		GET_JSON_NAME_VALUE_NOWARN(j, "phasemax", v.phasemax);
		GET_JSON_NAME_VALUE_NOWARN(j, "mask", v.mask);
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
	static double GetScale(const FrequencyValue& fv, uint32_t index, double timePass, double slow=5.0f) {
		auto t = 1.0f * slow / fv.storage.at(index).frequency;
		auto phase = fv.storage.at(index).phase;
		auto value = std::sin(timePass * 2.0f * pi / t + phase) + 1.0f;
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
    		GET_JSON_NAME_VALUE_NOWARN(wpj, "drag", drag);
    		GET_JSON_NAME_VALUE_NOWARN(wpj, "gravity", gravity);
			if(gravity.size() < 3) gravity.resize(3, gravity.at(0));
			Vector3f vecG(&gravity[0]);
			return [=](Particle& p, uint32_t, float life, float t){ 
				Vector3f acc = PM::GetDrag(p, drag) + vecG;
				PM::Accelerate(p, acc, t);
				PM::MoveByTime(p, t);
			};
		} else if(name == "angularmovement") {
			float drag {0.0f};
			Floats force {0.0f};
    		GET_JSON_NAME_VALUE_NOWARN(wpj, "drag", drag);
    		GET_JSON_NAME_VALUE_NOWARN(wpj, "force", force);
			if(force.size() < 3) force.resize(3, force.at(0));
			Vector3f vecF(&force[0]);
			return [=](Particle& p, uint32_t, float life, float t){ 
				Vector3f acc = PM::GetAngularDrag(p, drag) + vecF;
				PM::AngularAccelerate(p, acc, t);
				PM::RotateByTime(p, t);
			};
		} else if(name == "alphafade") {
			float fadeintime {0.5f}, fadeouttime {0.5f};
    		GET_JSON_NAME_VALUE_NOWARN(wpj, "fadeintime", fadeintime);
    		GET_JSON_NAME_VALUE_NOWARN(wpj, "fadeouttime", fadeouttime);
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
		} else if(name == "colorchange") {
			auto vc = VecChange::ReadFromJson(wpj);
			return [vc](Particle& p, uint32_t, float life, float t) {
				Vector3f result;
				for(int32_t i=0;i<3;i++)
					result[i] = FadeValueChange(life, vc.starttime, vc.endtime, vc.startvalue[i], vc.endvalue[i]);
				PM::MutiplyColor(p, result[0], result[1], result[2]);
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
			std::vector<Vector3f> lastMove;
			FrequencyValue fvx = FrequencyValue::ReadFromJson(wpj);
			std::vector<FrequencyValue> fxp = { fvx, fvx, fvx };
			return [=](Particle& p, uint32_t index, float life, float t) mutable {
				Vector3f pos {Vector3f::Zero()};
				for(int32_t i=0;i<3;i++) {
					if(fxp[0].mask[i] < 0.01) continue;
					FrequencyValue::CheckAndResize(fxp[i], index);
					FrequencyValue::GenFrequency(fxp[i], p, index, rf);
					pos[i] = FrequencyValue::GetScale(fxp[i], index, PM::LifetimePassed(p)) * 2.0f;
				}
				if(lastMove.size() <= index) lastMove.resize(2*(index+1), Vector3f::Zero());
				Vector3f& lastP = lastMove.at(index);
				PM::Move(p, pos[0] - lastP[0], pos[1] - lastP[1], 0);
				lastP = pos;
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
		std::memcpy(box.orgin, &wpe.origin[0], 3*sizeof(float));
		return ParticleBoxEmitterArgs::MakeEmittOp(box);
	} else if(wpe.name == "sphererandom") {
		ParticleSphereEmitterArgs sphere;
		sphere.emitSpeed = wpe.rate;
		sphere.minDistance = wpe.distancemin[0];
		sphere.maxDistance = wpe.distancemax[0];
		std::memcpy(sphere.directions, &wpe.directions[0], 3*sizeof(float));
		std::memcpy(sphere.orgin, &wpe.origin[0], 3*sizeof(float));
		std::memcpy(sphere.sign, &wpe.sign[0], 3*sizeof(int32_t));
		sphere.randomFn = rf;
		return ParticleSphereEmitterArgs::MakeEmittOp(sphere);
	} else 
		return [](std::vector<Particle>&, std::vector<ParticleInitOp>&, uint32_t, float) {};
}
