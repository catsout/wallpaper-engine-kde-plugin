#pragma once
#include "Particle.h"
#include <Eigen/Dense>
#include <cstdint>

namespace wallpaper {

class ParticleModify {
public:
	static void Move(Particle& p, float x, float y, float z) {
		p.position[0] += x;
		p.position[1] += y;
		p.position[2] += z;
	}
	static void MoveTo(Particle& p, float x, float y, float z) {
		p.position[0] = x;
		p.position[1] = y;
		p.position[2] = z;
	}
	static void MoveToNegZ(Particle& p) {
		p.position[2] = -std::abs(p.position[2]);
	}
	static void MoveByTime(Particle& p, float t) {
		Move(p, p.velocity[0] * t, p.velocity[1] * t, p.velocity[2] * t);
	}
	static void MoveMultiply(Particle& p, float x, float y, float z) {
		p.position[0] *= x;
		p.position[1] *= y;
		p.position[2] *= z;
	}
	static void MoveApplySign(Particle& p, int32_t x, int32_t y, int32_t z) {
		if(x != 0) {
			p.position[0] = std::abs(p.position[0]) * (float)x;
		}
		if(y != 0) {
			p.position[1] = std::abs(p.position[1]) * (float)y;
		}
		if(z != 0) {
			p.position[2] = std::abs(p.position[2]) * (float)z;
		}
	}
	static void SphereDirectOffset(Particle& p, Eigen::Vector3f base, float direct) {
		using namespace Eigen;
		Vector3f pos(p.position);
		Vector3f axis = base.cross(pos).normalized();
		Affine3f trans = Affine3f::Identity();
		trans.prerotate(AngleAxis<float>(direct, axis));
		pos = trans * pos;
		std::memcpy(p.position, pos.data(), 3*sizeof(float));
	}

	static void RotatePos(Particle& p, float x, float y, float z) {
		using namespace Eigen;
		Affine3f trans = Affine3f::Identity();

		trans.prerotate(AngleAxis<float>(y, Vector3f::UnitY())); // y
		trans.prerotate(AngleAxis<float>(x, Vector3f::UnitX())); // x
		trans.prerotate(AngleAxis<float>(-z, Vector3f::UnitZ())); // z
		Vector3f pos(p.position);
		pos = trans * pos;
		std::memcpy(p.position, pos.data(), 3*sizeof(float));
	}

	static void ChangeLifetime(Particle& p, float l) {
		p.lifetime += l;
	}
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

	static void InitLifetime(Particle& p, float l) {
		p.lifetime = l;
		p.lifetimeInit = l;
	}
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