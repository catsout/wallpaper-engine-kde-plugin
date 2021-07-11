#pragma once
#include "Particle.h"
#include <cstdint>

namespace wallpaper {

class ParticleModify {
public:
	static void Move(Particle&, float x, float y, float z);
	static void MoveTo(Particle&, float x, float y, float z);
	static void MoveToNegZ(Particle&); 
	static void MoveByTime(Particle&, float t);
	static void MoveApplySign(Particle&, int32_t x, int32_t y, int32_t z); 

	static void RotatePos(Particle&, float x, float y, float z);

	static void ChangeLifetime(Particle&, float l);
	static float LifetimePos(const Particle&);
	static double LifetimePassed(const Particle&);
	static bool LifetimeOk(const Particle&);

	static void ChangeColor(Particle&, float r, float g, float b);

	static void InitLifetime(Particle&, float);
	static void InitSize(Particle&, float);
	static void InitAlpha(Particle&, float);
	static void InitColor(Particle&, float r, float g, float b);

	static void InitVelocity(Particle&, float x, float y, float z);
	static void ChangeRotation(Particle&, float x, float y, float z);

	static void ChangeVelocity(Particle&, float x, float y, float z);
	static void Accelerate(Particle&, const std::vector<float> & acc, float t);
	static std::vector<float> GetDrag(Particle&, float s);
	static std::vector<float> GetAngularDrag(Particle&, float s);

	static void ChangeAngularVelocity(Particle&, float x, float y, float z);
	static void AngularAccelerate(Particle&, const std::vector<float> & acc, float t);
	static void Rotate(Particle&, float x, float y, float z);
	static void RotateByTime(Particle&, float t);

	static void MutiplyAlpha(Particle&, float);
	static void MutiplySize(Particle&, float);
	static void MutiplyColor(Particle&, float r, float g, float b);
	static void MutiplyVelocity(Particle&, float);

	static void MutiplyInitLifeTime(Particle&, float);
	static void MutiplyInitAlpha(Particle&, float);
	static void MutiplyInitSize(Particle&, float);
	static void MutiplyInitColor(Particle&, float r, float g, float b);

	static void Reset(Particle&);
};
}