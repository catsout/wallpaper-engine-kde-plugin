#pragma once
#include "Particle.h"
#include "Interface/IParticleInitializer.h"

#include <vector>
#include <random>
#include <memory>

namespace wallpaper {

enum class EmitterType {
	BOX,
	SPHERE
};

class ParticleEmitter {
public:
	ParticleEmitter(
		float minDistance,
		float maxDistance,
		float emitNumPerSecond,
		std::size_t maxcount,
		EmitterType type
	);
	~ParticleEmitter();
	uint32_t Emmit(std::vector<Particle>&, 
					const std::vector<std::shared_ptr<IParticleInitializer>>&);
	void TimePass(float time);

private:
	void Spwan(Particle& p,
					const std::vector<std::shared_ptr<IParticleInitializer>>&);
	float m_minDistance, m_maxDistance; 
	float m_emitNumPerSecond;
	std::size_t m_maxcount;
	EmitterType m_type;
	float m_time {0.0f};

	std::default_random_engine m_rSeed;
};
}