#pragma once
#include "Particle/ParticleEmitter.h"
#include "wpscene/WPParticleObject.h"

namespace wallpaper {
class WPParticleParser {
public:
	typedef std::function<float()> RandomFn;
	static ParticleInitOp genParticleInitOp(const nlohmann::json&, RandomFn);
	static ParticleOperatorOp genParticleOperatorOp(const nlohmann::json&, RandomFn);
	static ParticleEmittOp genParticleEmittOp(const wpscene::Emitter&, RandomFn);
	static ParticleInitOp genOverrideInitOp(const wpscene::ParticleInstanceoverride&);
};
}