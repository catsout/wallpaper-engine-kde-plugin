#include "ParticleEmitter.h"
#include "ParticleModify.h"
#include <random>
#include "Log.h"

using namespace wallpaper;

static float GetRandomIn(float min, float max, float random) {
	return min + (max - min)*random;
}

typedef std::function<Particle()> GenParticleOp;
typedef std::function<Particle()> SpwanOp;

int32_t FindLastParticle(const std::vector<Particle>& ps, int32_t last) {
	for(int32_t i=last;i<ps.size();i++) {
		if(!ParticleModify::LifetimeOk(ps[i]))
			return i;
	}
	return -1;
}
uint32_t GetEmitNum(double& timer, float speed) {
	double emitDur = 1.0f / speed;	
	if(emitDur > timer) return 0;
	uint32_t num = timer / emitDur;
	while(emitDur < timer) timer -= emitDur;
	if(timer < 0) timer = 0;
	return num;
}
uint32_t Emitt(std::vector<Particle>& particles, uint32_t num, uint32_t maxcount, SpwanOp Spwan) {
	int32_t lastPartcle = 0;
	uint32_t i = 0;
	for(i=0;i<num;i++) {
		lastPartcle = FindLastParticle(particles, lastPartcle);
		if(lastPartcle == -1) {
			lastPartcle = 0;
			if(maxcount == particles.size()) break;
			particles.push_back(Spwan());
		} else {
			particles[lastPartcle] = Spwan();
		}
	}
	return i + 1;
}

Particle Spwan(GenParticleOp gen, std::vector<ParticleInitOp>& inis) {
	auto particle = gen();
	for(auto& el:inis) el(particle);
	return particle;
}

ParticleEmittOp ParticleBoxEmitterArgs::MakeEmittOp(ParticleBoxEmitterArgs a) {
	double timer {0.0f};
	return [a, timer](std::vector<Particle>& ps, std::vector<ParticleInitOp>& inis, uint32_t maxcount, float timepass) mutable {
		timer += timepass;
		auto GenBox = [&]() {
			std::vector<float> pos;
			for(int32_t i=0;i<3;i++)
				pos.push_back(GetRandomIn(a.minDistance[i], a.maxDistance[i], (a.randomFn()-0.5f)*2.0f));
			auto p = Particle();
			ParticleModify::MoveTo(p, pos[0], pos[1], pos[2]);
			ParticleModify::Move(p, a.orgin[0], a.orgin[1], a.orgin[2]);
			return p;
		};
		Emitt(ps, GetEmitNum(timer, a.emitSpeed), maxcount, [&]() {
			return Spwan(GenBox, inis);
		});
	};
}

ParticleEmittOp ParticleSphereEmitterArgs::MakeEmittOp(ParticleSphereEmitterArgs a) {
	double timer {0.0f};
	return [a, timer](std::vector<Particle>& ps, std::vector<ParticleInitOp>& inis, uint32_t maxcount, float timepass) mutable {
		timer += timepass;
		auto GenSphere = [&]() {
			auto p = Particle();
			float radius = GetRandomIn(a.minDistance, a.maxDistance, a.randomFn());
			ParticleModify::MoveTo(p, radius, 0, 0);
			ParticleModify::RotatePos(p, 360*a.randomFn(), 360*a.randomFn(), 360*a.randomFn());
			ParticleModify::Move(p, a.orgin[0], a.orgin[1], a.orgin[2]);
			ParticleModify::MoveApplySign(p, a.sign[0], a.sign[1], a.sign[2]);
			return p;
		};
		Emitt(ps, GetEmitNum(timer, a.emitSpeed), maxcount, [&]() {
			return Spwan(GenSphere, inis);
		});
	};
}