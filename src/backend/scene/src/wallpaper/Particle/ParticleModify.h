#pragma once
#include "Particle.h"
#include <Eigen/Dense>
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
	static float inline LifetimePos(const Particle& p) {
		if (p.lifetime < 0)
			return 1.0f;
		return 1.0f - (p.lifetime / p.lifetimeInit);
	}
	static double LifetimePassed(const Particle&);
	static bool inline LifetimeOk(const Particle& p) {
		return p.lifetime > 0.0f;
	}

	static void ChangeColor(Particle&, float r, float g, float b);

	static void InitLifetime(Particle&, float);
	static void InitSize(Particle&, float);
	static void InitAlpha(Particle&, float);
	static void InitColor(Particle&, float r, float g, float b);

	static void InitVelocity(Particle&, float x, float y, float z);
	static void ChangeRotation(Particle&, float x, float y, float z);

	static void inline ChangeVelocity(Particle& p, float x, float y, float z) {
		p.velocity[0] += x;
		p.velocity[1] += y;
		p.velocity[2] += z;
	}
	static void inline Accelerate(Particle& p, const Eigen::Vector3f& acc, float t) {
		ChangeVelocity(p, acc[0]*t, acc[1]*t, acc[2]*t);
	}

	static Eigen::Vector3f GetDrag(Particle&, float s);
	static Eigen::Vector3f GetAngularDrag(Particle&, float s);

	static void ChangeAngularVelocity(Particle&, float x, float y, float z);
	static void AngularAccelerate(Particle&, const Eigen::Vector3f& acc, float t);
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