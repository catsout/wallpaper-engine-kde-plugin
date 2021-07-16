#include "ParticleModify.h"
#include "Log.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace wallpaper;
using namespace Eigen;

void ParticleModify::Move(Particle &p, float x, float y, float z) {
	p.position[0] += x;
	p.position[1] += y;
	p.position[2] += z;
}

void ParticleModify::MoveTo(Particle &p, float x, float y, float z) {
	p.position[0] = x;
	p.position[1] = y;
	p.position[2] = z;
}
void ParticleModify::MoveToNegZ(Particle& p) {
	p.position[2] = -std::abs(p.position[2]);
}

void ParticleModify::MoveByTime(Particle &p, float t) {
	Move(p, p.velocity[0] * t, p.velocity[1] * t, p.velocity[2] * t);
}


void ParticleModify::MoveApplySign(Particle& p, int32_t x, int32_t y, int32_t z) {
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


void ParticleModify::RotatePos(Particle &p, float x, float y, float z) {
	Affine3f trans = Affine3f::Identity();

	trans.prerotate(AngleAxis<float>(y, Vector3f::UnitY())); // y
	trans.prerotate(AngleAxis<float>(x, Vector3f::UnitX())); // x
	trans.prerotate(AngleAxis<float>(-z, Vector3f::UnitZ())); // z
	Vector3f pos(p.position);
	pos = trans * pos;
	std::memcpy(p.position, pos.data(), 3*sizeof(float));
}

void ParticleModify::InitColor(Particle &p, float r, float g, float b) {
	p.color[0] = r;
	p.color[1] = g;
	p.color[2] = b;
	std::memcpy(p.colorInit, p.color, 3 * sizeof(float));
}
void ParticleModify::ChangeColor(Particle &p, float r, float g, float b) {
	p.color[0] += r;
	p.color[1] += g;
	p.color[2] += b;
}

void ParticleModify::ChangeLifetime(Particle &p, float l) {
	p.lifetime += l;
}
void ParticleModify::InitLifetime(Particle &p, float l) {
	p.lifetime = l;
	p.lifetimeInit = l;
}
void ParticleModify::InitSize(Particle &p, float s) {
	p.size = s;
	p.sizeInit = s;
}
void ParticleModify::InitAlpha(Particle &p, float a) {
	p.alpha = a;
	p.alphaInit = a;
}

void ParticleModify::InitVelocity(Particle &p, float x, float y, float z) {
	p.velocity[0] = x;
	p.velocity[1] = y;
	p.velocity[2] = z;
}


void ParticleModify::ChangeRotation(Particle &p, float x, float y, float z) {
	p.rotation[0] += x;
	p.rotation[1] += y;
	p.rotation[2] += z;
}

void ParticleModify::MutiplyAlpha(Particle &p, float x) {
	p.alpha *= x;
}
void ParticleModify::MutiplySize(Particle &p, float x) {
	p.size *= x;
}
void ParticleModify::MutiplyColor(Particle &p, float r, float g, float b) {
	p.color[0] *= r;
	p.color[1] *= g;
	p.color[2] *= b;
}

void ParticleModify::MutiplyVelocity(Particle& p, float m) {
	p.velocity[0] *= m;	
	p.velocity[1] *= m;	
	p.velocity[2] *= m;	
}

double ParticleModify::LifetimePassed(const Particle &p) {
	return p.lifetimeInit - p.lifetime;
}

Eigen::Vector3f ParticleModify::GetDrag(Particle& p, float s) {
	Eigen::Vector3f result {Eigen::Vector3f::Zero()};
	if(s != 0) {
		result = Eigen::Vector3f(p.velocity) * (-s);
	}
	return result;
}

Eigen::Vector3f ParticleModify::GetAngularDrag(Particle &p, float s) {
	Eigen::Vector3f result {Eigen::Vector3f::Zero()};
	if(s != 0) {
		result = Eigen::Vector3f(p.velocity) * (-s);
	}
	return result;
}

void ParticleModify::ChangeAngularVelocity(Particle &p, float x, float y, float z) {
	p.angularVelocity[0] += x;
	p.angularVelocity[1] += y;
	p.angularVelocity[2] += z;
}
void ParticleModify::AngularAccelerate(Particle &p, const Vector3f&acc, float t) {
	ChangeAngularVelocity(p, acc[0]*t, acc[1]*t, acc[2]*t);
}

void ParticleModify::Rotate(Particle& p, float x, float y, float z) {
	p.rotation[0] += x;
	p.rotation[1] += y;
	p.rotation[2] += z;
}
void ParticleModify::RotateByTime(Particle &p, float t) {
	Rotate(p, p.angularVelocity[0] * t, p.angularVelocity[1] * t, p.angularVelocity[2] * t);
}

void ParticleModify::MutiplyInitLifeTime(Particle& p, float m) {
	p.lifetime *= m;
	p.lifetimeInit = p.lifetimeInit;
}
void ParticleModify::MutiplyInitAlpha(Particle& p, float m) {
	p.alpha *= m;
	p.alphaInit = p.alpha;
}
void ParticleModify::MutiplyInitSize(Particle& p, float m) {
	p.size *= m;
	p.sizeInit = p.size;
}
void ParticleModify::MutiplyInitColor(Particle& p, float r, float g, float b) {
	MutiplyColor(p, r, g, b);	
	std::memcpy(p.colorInit, p.color, 3*sizeof(float));
}


void ParticleModify::Reset(Particle &p) {
	p.alpha = p.alphaInit;
	p.size = p.sizeInit;
	std::memcpy(p.color, p.colorInit, 3 * sizeof(float));
}