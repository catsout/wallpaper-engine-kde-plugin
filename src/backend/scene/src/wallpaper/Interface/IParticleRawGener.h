#pragma once
#include <vector>
#include <functional>

#include "Particle/Particle.h"
#include "SceneMesh.h"

namespace wallpaper {
struct ParticleRawGenSpec {
	float* lifetime;	
};
typedef std::function<void(const Particle&, const ParticleRawGenSpec&)> ParticleRawGenSpecOp;

class IParticleRawGener {
public:
	IParticleRawGener() = default;
	virtual ~IParticleRawGener() = default;

	virtual void GenGLData(const std::vector<Particle>&, SceneMesh&, ParticleRawGenSpecOp&) = 0; 
};
}