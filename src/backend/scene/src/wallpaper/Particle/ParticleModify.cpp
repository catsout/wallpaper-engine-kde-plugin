#include "ParticleModify.h"
#include "Log.h"
#include <cstring>

using namespace wallpaper;

void ParticleModify::Move(Particle& p, float x, float y, float z) {
	p.position[0] += x;
	p.position[1] += y;
	p.position[2] += z;
}

void ParticleModify::MoveTo(Particle& p, float x, float y, float z) {
	p.position[0] = x;
	p.position[1] = y;
	p.position[2] = z;
}


void ParticleModify::MoveByTime(Particle& p, float t) {
	Move(p,p.velocity[0]*t, p.velocity[1]*t, p.velocity[2]*t);
}

void ParticleModify::InitColor(Particle& p, float r, float g, float b) {
	p.color[0] = r;	
	p.color[1] = g;	
	p.color[2] = b;	
	std::memcpy(p.colorInit, p.color, 3*sizeof(float));
}

void ParticleModify::ChangeLifetime(Particle& p, float l) {
	p.lifetime += l;
} 
void ParticleModify::InitLifetime(Particle& p, float l) {
	p.lifetime = l;	
	p.lifetimeInit = l;
}
void ParticleModify::InitSize(Particle& p, float s) {
	p.size = s;
	p.sizeInit = s;
}
void ParticleModify::InitAlpha(Particle& p, float a) {
	p.alpha = a;
	p.alphaInit = a;
}

void ParticleModify::InitVelocity(Particle& p, float x, float y, float z) {
	p.velocity[0] = x;
	p.velocity[1] = y;
	p.velocity[2] = z;
}


void ParticleModify::MutiplyAlpha(Particle& p, float x) {
	p.alpha *= x;
}
void ParticleModify::MutiplySize(Particle& p, float x) {
	p.size *= x;
}

float ParticleModify::LifetimePos(const Particle& p) {
	if(p.lifetime < 0) return 1.0f;
	return 1.0f - (p.lifetime / p.lifetimeInit);
}

bool ParticleModify::LifetimeOk(const Particle& p) {
	return p.lifetime > 0.0f;
}


void ParticleModify::Reset(Particle& p) {
	p.alpha = p.alphaInit;
	p.size = p.sizeInit;
	std::memcpy(p.color, p.colorInit, 3*sizeof(float));
}