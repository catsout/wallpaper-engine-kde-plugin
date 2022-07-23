#pragma once
#include "Interface/IParticleRawGener.h"
#include <functional>

namespace wallpaper
{

class WPParticleRawGener : public IParticleRawGener {
public:
    WPParticleRawGener() {};
    virtual ~WPParticleRawGener() {};

    virtual void GenGLData(const std::vector<Particle>&, SceneMesh&, ParticleRawGenSpecOp&);
};

} // namespace wallpaper