#include "ParticleInitializer.h"
#include <cstring>
#include "Log.h"

using namespace wallpaper;
static std::uniform_real_distribution<float> ur(0.0f, 1.0f);

static float GetRandomIn(float min, float max, float random) {
	return min + (max - min)*random;
}

std::vector<float> GetRandomIn(const std::vector<float>& min, const std::vector<float>& max, float random) {
	std::vector<float> result;
	for(int32_t i=0;i<min.size() && i<max.size();i++) {
		result.push_back(GetRandomIn(min.at(i), max.at(i), random));
	}
	return result;
}

ParticleInitLifeTime::ParticleInitLifeTime(float min, float max):m_min(min), m_max(max) {};
void ParticleInitLifeTime::InitParticle(std::default_random_engine& rs, Particle& p) {
	p.lifetime = GetRandomIn(m_min, m_max, ur(rs));
}

ParticleInitSize::ParticleInitSize(float min, float max):m_min(min), m_max(max) {};
void ParticleInitSize::InitParticle(std::default_random_engine& rs, Particle& p) {
	p.size = GetRandomIn(m_min, m_max, ur(rs));
}

ParticleInitColor::ParticleInitColor(const std::vector<float>& min, const std::vector<float>& max):m_min(min), m_max(max) {};
void ParticleInitColor::InitParticle(std::default_random_engine& rs, Particle& p) {
	auto result = GetRandomIn(m_min, m_max, ur(rs));
	std::memcpy(p.color, &result[0], 3*sizeof(float));
}

ParticleInitAlpha::ParticleInitAlpha(float min, float max):m_min(min), m_max(max) {};
void ParticleInitAlpha::InitParticle(std::default_random_engine& rs, Particle& p) {
	p.alpha = GetRandomIn(m_min, m_max, ur(rs));
}

ParticleInitRotation::ParticleInitRotation(const std::vector<float>& min, const std::vector<float>& max):m_min(min), m_max(max) {};
void ParticleInitRotation::InitParticle(std::default_random_engine& rs, Particle& p) {
	auto result = GetRandomIn(m_min, m_max, ur(rs));
	std::memcpy(p.rotation, &result[0], 3*sizeof(float));
}

ParticleInitVelocity::ParticleInitVelocity(const std::vector<float>& min, const std::vector<float>& max):m_min(min), m_max(max) {};
void ParticleInitVelocity::InitParticle(std::default_random_engine& rs, Particle& p) {
	auto result = GetRandomIn(m_min, m_max, ur(rs));
	std::memcpy(p.velocity, &result[0], 3*sizeof(float));
}

ParticleInitAngularVelocity::ParticleInitAngularVelocity(const std::vector<float>& min, const std::vector<float>& max):m_min(min), m_max(max) {};
void ParticleInitAngularVelocity::InitParticle(std::default_random_engine& rs, Particle& p) {
	auto result = GetRandomIn(m_min, m_max, ur(rs));
	std::memcpy(p.angularVelocity, &result[0], 3*sizeof(float));
}