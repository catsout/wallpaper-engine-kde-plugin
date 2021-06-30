#pragma once
#include "Particle/Particle.h" 
#include <random>

namespace wallpaper {

class IParticleInitializer {
public:
	IParticleInitializer() = default;
	virtual ~IParticleInitializer() = default;

	virtual void InitParticle(std::default_random_engine&, Particle&) = 0;
};
}