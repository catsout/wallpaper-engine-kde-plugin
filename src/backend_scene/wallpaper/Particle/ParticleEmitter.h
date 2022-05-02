#pragma once
#include "Particle.h"

#include <vector>
#include <random>
#include <memory>
#include <functional>
#include <array>

#include "Utils/span.hpp"

namespace wallpaper
{

struct ParticleInfo {
    Span<Particle> particles;
    double         time;
    double         time_pass;
};

typedef std::function<void(Particle&, double)> ParticleInitOp;
// particle index lifetime-percent passTime
typedef std::function<void(const ParticleInfo&)> ParticleOperatorOp;

typedef std::function<void(std::vector<Particle>&, std::vector<ParticleInitOp>&, uint32_t maxcount,
                           double timepass)>
    ParticleEmittOp;

struct ParticleBoxEmitterArgs {
    std::array<float, 3>   directions;
    std::array<float, 3>   minDistance;
    std::array<float, 3>   maxDistance;
    float                  emitSpeed;
    std::array<float, 3>   orgin;
    std::function<float()> randomFn;

    static ParticleEmittOp MakeEmittOp(ParticleBoxEmitterArgs);
};

struct ParticleSphereEmitterArgs {
    std::array<float, 3>   directions;
    float                  minDistance;
    float                  maxDistance;
    float                  emitSpeed;
    std::array<float, 3>   orgin;
    std::array<int32_t, 3> sign;
    std::function<float()> randomFn;

    static ParticleEmittOp MakeEmittOp(ParticleSphereEmitterArgs);
};

} // namespace wallpaper