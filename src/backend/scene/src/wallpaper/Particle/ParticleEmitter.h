#pragma once
#include "Particle.h"

#include <vector>
#include <random>
#include <memory>
#include <functional>

namespace wallpaper {

enum class EmitterType {
	BOX,
	SPHERE
};

typedef std::function<void(Particle&)> ParticleInitOp;
// particle index lifetime-percent passTime
typedef std::function<void(Particle&, uint32_t, float, float)> ParticleOperatorOp;

class ParticleEmitter {
public:
	ParticleEmitter(
		float minDistance,
		float maxDistance,
		float emitNumPerSecond,
		std::size_t maxcount,
		EmitterType type,
		std::function<float()> randomFn
	);
	~ParticleEmitter();
	ParticleEmitter(ParticleEmitter&) = delete;	
	ParticleEmitter(ParticleEmitter&&) = delete;	

	uint32_t Emmit(std::vector<Particle>&, std::vector<ParticleInitOp>&);
	void TimePass(float time);

private:
	void Spwan(Particle& p, std::vector<ParticleInitOp>&);
	float m_minDistance, m_maxDistance; 
	float m_emitNumPerSecond;
	std::size_t m_maxcount;
	EmitterType m_type;
	float m_time {0.0f};

	std::function<float()> m_randomFn;
};
}