#pragma once
#include "Interface/IParticleRawGener.h"

namespace wallpaper {
class WPParticleRawGener : public IParticleRawGener {

public:
	WPParticleRawGener() {};
	virtual ~WPParticleRawGener() {};

	virtual std::vector<float> GenGLData(const Particle&, const SceneVertexArray&);
};

}