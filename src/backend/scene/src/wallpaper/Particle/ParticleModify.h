#pragma once
#include "Particle.h"
#include <Eigen/Dense>
#include <cstdint>

namespace wallpaper {

namespace ParticleModify {
	inline void Move(Particle& p, double x, double y, double z) noexcept {
		p.position[0] += x;
		p.position[1] += y;
		p.position[2] += z;
	}
	inline void MoveTo(Particle& p, double x, double y, double z) noexcept {
		p.position[0] = x;
		p.position[1] = y;
		p.position[2] = z;
	}
	inline void MoveToNegZ(Particle& p) noexcept {
		p.position[2] = -std::abs(p.position[2]);
	}
	inline void MoveByTime(Particle& p, double t) noexcept {
		Move(p, p.velocity[0] * t, p.velocity[1] * t, p.velocity[2] * t);
	}
	inline void MoveMultiply(Particle& p, double x, double y, double z) noexcept {
		p.position[0] *= x;
		p.position[1] *= y;
		p.position[2] *= z;
	}
	inline void MoveApplySign(Particle& p, int32_t x, int32_t y, int32_t z) noexcept {
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
	inline void SphereDirectOffset(Particle& p, Eigen::Vector3f base, float direct) noexcept {
		using namespace Eigen;
		Vector3f pos(p.position);
		Vector3f axis = base.cross(pos).normalized();
		Affine3f trans = Affine3f::Identity();
		trans.prerotate(AngleAxis<float>(direct, axis));
		pos = trans * pos;
		std::memcpy(p.position, pos.data(), 3*sizeof(float));
	}

	inline void RotatePos(Particle& p, float x, float y, float z) noexcept {
		using namespace Eigen;
		Affine3f trans = Affine3f::Identity();

		trans.prerotate(AngleAxis<float>(y, Vector3f::UnitY())); // y
		trans.prerotate(AngleAxis<float>(x, Vector3f::UnitX())); // x
		trans.prerotate(AngleAxis<float>(-z, Vector3f::UnitZ())); // z
		Vector3f pos(p.position);
		pos = trans * pos;
		std::memcpy(p.position, pos.data(), 3*sizeof(float));
	}

	inline void ChangeLifetime(Particle& p, double l) noexcept {
		p.lifetime += l;
	}
	inline double LifetimePos(const Particle& p) {
		if (p.lifetime < 0)
			return 1.0f;
		return 1.0f - (p.lifetime / p.lifetimeInit);
	}
	inline double LifetimePassed(const Particle &p) noexcept {
		return p.lifetimeInit - p.lifetime;
	}
	inline bool LifetimeOk(const Particle& p) noexcept {
		return p.lifetime > 0.0f;
	}

	void ChangeColor(Particle&, float r, float g, float b);

	inline void InitLifetime(Particle& p, float l) noexcept {
		p.lifetime = l;
		p.lifetimeInit = l;
	}
	void InitSize(Particle&, float);
	void InitAlpha(Particle&, float);
	void InitColor(Particle&, float r, float g, float b);

	void InitVelocity(Particle&, float x, float y, float z);
	void ChangeRotation(Particle&, float x, float y, float z);

	inline void ChangeVelocity(Particle& p, double x, double y, double z) noexcept {
		p.velocity[0] += x;
		p.velocity[1] += y;
		p.velocity[2] += z;
	}
	inline void Accelerate(Particle& p, const Eigen::Vector3d& acc, double t) noexcept {
		ChangeVelocity(p, acc[0]*t, acc[1]*t, acc[2]*t);
	}

	inline Eigen::Vector3f GetPos(Particle& p) {
		return Eigen::Vector3f(p.position);
	}
	inline Eigen::Vector3f GetVelocity(Particle& p) {
		return Eigen::Vector3f(p.velocity);
	}
	inline Eigen::Vector3f GetAngular(Particle& p) {
		return Eigen::Vector3f(p.rotation);
	}

	inline void ChangeAngularVelocity(Particle& p, double x, double y, double z) noexcept {
		p.angularVelocity[0] += x;
		p.angularVelocity[1] += y;
		p.angularVelocity[2] += z;
	}
	inline void AngularAccelerate(Particle& p, const Eigen::Vector3d& acc, double t) noexcept {
		ChangeAngularVelocity(p, acc[0]*t, acc[1]*t, acc[2]*t);
	}
	inline void Rotate(Particle& p, double x, double y, double z) noexcept {
		p.rotation[0] += x;
		p.rotation[1] += y;
		p.rotation[2] += z;
	}
	inline void RotateByTime(Particle& p, double t) noexcept {
		Rotate(p, p.angularVelocity[0] * t, p.angularVelocity[1] * t, p.angularVelocity[2] * t);
	}

	void MutiplyAlpha(Particle&, float);
	void MutiplySize(Particle&, float);
	void MutiplyColor(Particle&, float r, float g, float b);
	void MutiplyVelocity(Particle&, float);

	void MutiplyInitLifeTime(Particle&, float);
	void MutiplyInitAlpha(Particle&, float);
	void MutiplyInitSize(Particle&, float);
	void MutiplyInitColor(Particle&, float r, float g, float b);

	void Reset(Particle&);
};
}