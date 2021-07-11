#pragma once
#include <vector>
#include "Particle/Particle.h"
#include "SceneMesh.h"

namespace wallpaper {
class IParticleRawGener {
public:
	IParticleRawGener() = default;
	virtual ~IParticleRawGener() = default;
	virtual void GenGLData(const std::vector<Particle>&, SceneMesh&) = 0; 
};
}