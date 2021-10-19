#include "ParticleModify.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace wallpaper;
using namespace Eigen;


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