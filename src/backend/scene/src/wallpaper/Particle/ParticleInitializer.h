#pragma once
#include "Particle.h"
#include "Interface/IParticleInitializer.h"

#include <vector>
#include <random>

namespace wallpaper {
class ParticleInitLifeTime : public IParticleInitializer {
public:
	ParticleInitLifeTime(float min, float max);
	virtual ~ParticleInitLifeTime() = default;
	void InitParticle(std::default_random_engine&, Particle&) override;
private:
	float m_min, m_max;
};

class ParticleInitSize : public IParticleInitializer {
public:
	ParticleInitSize(float min, float max);
	virtual ~ParticleInitSize() = default;
	void InitParticle(std::default_random_engine&, Particle&) override;
private:
	float m_min, m_max;
};

class ParticleInitColor : public IParticleInitializer {
public:
	ParticleInitColor(const std::vector<float>& min, const std::vector<float>& max);
	virtual ~ParticleInitColor() = default;
	void InitParticle(std::default_random_engine&, Particle&) override;
private:
	std::vector<float> m_min, m_max;
};

class ParticleInitAlpha : public IParticleInitializer {
public:
	ParticleInitAlpha(float min, float max);
	virtual ~ParticleInitAlpha() = default;
	void InitParticle(std::default_random_engine&, Particle&) override;
private:
	float m_min, m_max;
};

class ParticleInitRotation : public IParticleInitializer {
public:
	ParticleInitRotation(const std::vector<float>& min, const std::vector<float>& max);
	virtual ~ParticleInitRotation() = default;
	void InitParticle(std::default_random_engine&, Particle&) override;
private:
	std::vector<float> m_min, m_max;
};

class ParticleInitVelocity : public IParticleInitializer {
public:
	ParticleInitVelocity(const std::vector<float>& min, const std::vector<float>& max);
	virtual ~ParticleInitVelocity() = default;
	void InitParticle(std::default_random_engine&, Particle&) override;
private:
	std::vector<float> m_min, m_max;
};

class ParticleInitAngularVelocity : public IParticleInitializer {
public:
	ParticleInitAngularVelocity(const std::vector<float>& min, const std::vector<float>& max);
	virtual ~ParticleInitAngularVelocity() = default;
	void InitParticle(std::default_random_engine&, Particle&) override;
private:
	std::vector<float> m_min, m_max;
};


}