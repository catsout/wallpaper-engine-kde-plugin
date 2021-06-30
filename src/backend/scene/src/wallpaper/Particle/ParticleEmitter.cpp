#include "ParticleEmitter.h"
#include <random>
#include "Log.h"

using namespace wallpaper;
static std::uniform_real_distribution<float> ur(0.0f, 1.0f);

static float GetRandomIn(float min, float max, float random) {
	return min + (max - min)*random;
}

void GenBox(Particle& p, float min, float max, std::default_random_engine& rSeed) {
	p.position[0] = GetRandomIn(min, max, ur(rSeed));
	p.position[1] = GetRandomIn(min, max, ur(rSeed));
}

void ParticleEmitter::Spwan(Particle& p, 
					const std::vector<std::shared_ptr<IParticleInitializer>>& initializers) {
	p = Particle();
	GenBox(p, m_minDistance, m_maxDistance, m_rSeed);
	for(auto& el:initializers) {
		el->InitParticle(m_rSeed, p);
	}
}

ParticleEmitter::ParticleEmitter(
	float minDistance,
	float maxDistance,
	float emitNumPerSecond,
	std::size_t maxcount,
	EmitterType type
):  m_minDistance(minDistance),
	m_maxDistance(maxDistance),
	m_emitNumPerSecond(emitNumPerSecond),
	m_maxcount(maxcount),
	m_type(type) {}
ParticleEmitter::~ParticleEmitter() {}

int32_t FindLastParticle(const std::vector<Particle>& ps, int32_t last) {
	for(int32_t i=last;i<ps.size();i++)
		if(ps.at(i).lifetime <= 0.0f) {
			return i;
		}
	return -1;
}

uint32_t ParticleEmitter::Emmit(std::vector<Particle>& particles, 
					const std::vector<std::shared_ptr<IParticleInitializer>>& initializers) {
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