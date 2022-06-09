#pragma once
#include "Particle/ParticleEmitter.h"
#include "wpscene/WPParticleObject.h"

namespace wallpaper
{
class WPParticleParser {
public:
    typedef std::function<float()> RandomFn;

    static ParticleInitOp     genParticleInitOp(const nlohmann::json&, RandomFn);
    static ParticleOperatorOp genParticleOperatorOp(const nlohmann::json&, RandomFn,
                                                    const wpscene::ParticleInstanceoverride&);
    static ParticleEmittOp genParticleEmittOp(const wpscene::Emitter&, RandomFn, bool sort = false);
    static ParticleInitOp  genOverrideInitOp(const wpscene::ParticleInstanceoverride&);
};
} // namespace wallpaper