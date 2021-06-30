#pragma once
#include <vector>
#include "Particle/Particle.h"
#include "SceneVertexArray.h"

namespace wallpaper {
class IParticleRawGener {
public:
	IParticleRawGener() = default;
	virtual ~IParticleRawGener() = default;
	virtual std::vector<float> GenGLData(const Particle&, const SceneVertexArray&) = 0;
};
}