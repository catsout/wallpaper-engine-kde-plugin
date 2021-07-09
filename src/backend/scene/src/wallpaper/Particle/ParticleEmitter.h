#pragma once
#include "Particle.h"

#include <vector>
#include <random>
#include <memory>
#include <functional>

namespace wallpaper {

typedef std::function<void(Particle&)> ParticleInitOp;
// particle index lifetime-percent passTime
typedef std::function<void(Particle&, uint32_t, float, float)> ParticleOperatorOp;

typedef std::function<void(std::vector<Particle>&, std::vector<ParticleInitOp>&, uint32_t maxcount, float timepass)> ParticleEmittOp;

struct ParticleBoxEmitterArgs {
	float directions[3];
	float minDistance[3];
	float maxDistance[3];
	float emitSpeed;
	float orgin[3];
	std::function<float()> randomFn;

	static ParticleEmittOp MakeEmittOp(ParticleBoxEmitterArgs);
};

struct ParticleSphereEmitterArgs {
	float directions[3];
	float minDistance;
	float maxDistance;
	float emitSpeed;
	float orgin[3];
	std::function<float()> randomFn;

	static ParticleEmittOp MakeEmittOp(ParticleSphereEmitterArgs);
};

}