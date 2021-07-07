#include "ParticleEmitter.h"
#include "ParticleModify.h"
#include <random>
#include "Log.h"

using namespace wallpaper;

static float GetRandomIn(float min, float max, float random) {
	return min + (max - min)*random;
}

void GenBox(Particle& p, float min, float max, std::function<float()>& rfunc) {
	auto gen = [&]() { return GetRandomIn(min, max, rfunc()); };
	ParticleModify::MoveTo(p, gen(), gen(), 0);
}

void ParticleEmitter::Spwan(Particle& p, std::vector<ParticleInitOp>& initializers) {
	p = Particle();
	GenBox(p, m_minDistance, m_maxDistance, m_randomFn);
	for(auto& el:initializers) {
		el(p);
	}
}

ParticleEmitter::ParticleEmitter(
	float minDistance,
	float maxDistance,
	float emitNumPerSecond,
	std::size_t maxcount,
	EmitterType type,
	std::function<float()> randomFn
):  m_minDistance(minDistance),
	m_maxDistance(maxDistance),
	m_emitNumPerSecond(emitNumPerSecond),
	m_maxcount(maxcount),
	m_type(type),
	m_randomFn(randomFn) {}
ParticleEmitter::~ParticleEmitter() {}

int32_t FindLastParticle(const std::vector<Particle>& ps, int32_t last) {
	for(int32_t i=last;i<ps.size();i++)
		if(ps.at(i).lifetime <= 0.0f) {
			return i;
		}
	return -1;
}

uint32_t ParticleEmitter::Emmit(std::vector<Particle>& particles, std::vector<ParticleInitOp>& initializers) {
	int32_t lastPartcle = 0;
	float timeEmit = 1.0f / m_emitNumPerSecond;
	if(timeEmit > m_time) return 0;
	int32_t num = m_time / timeEmit;
	while(timeEmit < m_time) m_time -= timeEmit;
	if(m_time < 0) m_time = 0;
	for(int32_t i=0;i<num;i++) {
		lastPartcle = FindLastParticle(particles, lastPartcle);
		if(lastPartcle == -1) {
			lastPartcle = 0;
			if(m_maxcount == particles.size()) return 0;
			particles.push_back(Particle());
			Spwan(particles.back(), initializers);
		} else {
			Spwan(particles.at(lastPartcle), initializers);
		}
	}
	return num;
}

void ParticleEmitter::TimePass(float time) {
	m_time += time;
}