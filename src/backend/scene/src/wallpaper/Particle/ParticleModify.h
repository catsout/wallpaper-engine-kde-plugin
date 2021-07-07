#pragma once
#include "Particle.h"

namespace wallpaper {

class ParticleModify {
public:
	static void Move(Particle&, float x, float y, float z);
	static void MoveTo(Particle&, float x, float y, float z);
	static void MoveByTime(Particle&, float t);

	static void ChangeLifetime(Particle&, float l);
	static float LifetimePos(const Particle&);
	static bool LifetimeOk(const Particle&);

	static void InitColor(Particle&, float r, float g, float b);
	static void InitLifetime(Particle&, float l);
	static void InitSize(Particle&, float);
	static void InitAlpha(Particle&, float);
	static void InitVelocity(Particle&, float x, float y, float z);

	static void MutiplyAlpha(Particle&, float);
	static void MutiplySize(Particle&, float);
	static void Reset(Particle&);
};
}