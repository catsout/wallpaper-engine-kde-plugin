#pragma once
#include "Particle.h"

namespace wallpaper {

class ParticleModify {
public:
	static void Move(Particle&, float x, float y, float z);
	static void MoveTo(Particle&, float x, float y, float z);
	static void MoveByTime(Particle&, float t);

	static void RotatePos(Particle&, float x, float y, float z);

	static void ChangeLifetime(Particle&, float l);
	static float LifetimePos(const Particle&);
	static double LifetimePassed(const Particle&);
	static bool LifetimeOk(const Particle&);

	static void InitColor(Particle&, float r, float g, float b);
	static void InitLifetime(Particle&, float l);
	static void InitSize(Particle&, float);
	static void InitAlpha(Particle&, float);
	static void InitVelocity(Particle&, float x, float y, float z);
	static void InitRotation(Particle&, float x, float y, float z);
	static void InitAngularVelocity(Particle&, float x, float y, float z);

	static void ChangeVelocity(Particle&, float x, float y, float z);
	static void Accelerate(Particle&, const std::vector<float> & acc, float t);

	static void ChangeAngularVelocity(Particle&, float x, float y, float z);
	static void AngularAccelerate(Particle&, const std::vector<float> & acc, float t);
	static void Rotate(Particle&, float x, float y, float z);
	static void RotateByTime(Particle&, float t);

	static void MutiplyAlpha(Particle&, float);
	static void MutiplySize(Particle&, float);
	static void Reset(Particle&);
};
}