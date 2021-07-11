#pragma once
#include "Interface/IParticleRawGener.h"

namespace wallpaper {
class WPParticleRawGener : public IParticleRawGener {

public:
	WPParticleRawGener() {};
	virtual ~WPParticleRawGener() {};

	virtual void GenGLData(const std::vector<Particle>&, SceneMesh&);
};

}