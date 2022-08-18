#pragma once
#include <span>
#include <memory>
#include <functional>

#include "Particle/Particle.h"
#include "Scene/SceneMesh.h"

namespace wallpaper
{
struct ParticleRawGenSpec {
    float* lifetime;
};
using ParticleRawGenSpecOp = std::function<void(const Particle&, const ParticleRawGenSpec&)>;

class ParticleInstance;
class IParticleRawGener {
public:
    IParticleRawGener()          = default;
    virtual ~IParticleRawGener() = default;

    virtual void GenGLData(std::span<const std::unique_ptr<ParticleInstance>>, SceneMesh&,
                           ParticleRawGenSpecOp&) = 0;
};
} // namespace wallpaper
