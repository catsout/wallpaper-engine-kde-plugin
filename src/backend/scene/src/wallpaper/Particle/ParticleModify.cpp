#include "ParticleModify.h"
#include "Log.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace wallpaper;

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

void ParticleModify::MoveByTime(Particle &p, float t) {
	Move(p, p.velocity[0] * t, p.velocity[1] * t, p.velocity[2] * t);
}


void ParticleModify::RotatePos(Particle &p, float x, float y, float z) {
	glm::mat4 rotate(1.0f);
	rotate = glm::rotate(rotate, glm::radians(x), glm::vec3(1.0, 0.0, 0.0));
	rotate = glm::rotate(rotate, glm::radians(y), glm::vec3(0.0, 1.0, 0.0));
	rotate = glm::rotate(rotate, glm::radians(z), glm::vec3(0.0, 0.0, 1.0));
	glm::vec3 pos = glm::make_vec3(p.position);
	pos = rotate * glm::vec4(pos, 1.0f);
	std::memcpy(p.position, glm::value_ptr(pos), 3*sizeof(float));
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

void ParticleModify::InitVelocity(Particle &p, float x, float y, float z, float mutiply) {
	p.velocity[0] = x * mutiply;
	p.velocity[1] = y * mutiply;
	p.velocity[2] = z * mutiply;
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

float ParticleModify::LifetimePos(const Particle &p) {
	if (p.lifetime < 0)
		return 1.0f;
	return 1.0f - (p.lifetime / p.lifetimeInit);
}

double ParticleModify::LifetimePassed(const Particle &p) {
	return p.lifetimeInit - p.lifetime;
}

bool ParticleModify::LifetimeOk(const Particle &p) {
	return p.lifetime > 0.0f;
}

void ParticleModify::ChangeVelocity(Particle &p, float x, float y, float z, float mutiply) {
	p.velocity[0] += x * mutiply;
	p.velocity[1] += y * mutiply;
	p.velocity[2] += z * mutiply;
}
void ParticleModify::Accelerate(Particle &p, const std::vector<float> &acc, float t) {
	if(acc.size() != 3) return;
	ChangeVelocity(p, acc[0]*t, acc[1]*t, acc[2]*t);
}
std::vector<float> ParticleModify::GetDrag(Particle& p, float s) {
	std::vector<float> velocity(p.velocity, p.velocity + 3);
	/*
	double size = std::sqrt(std::accumulate(velocity.begin(), velocity.end(), 0.0f, [](double v1, float v2) {
		return v1 + v2*v2;	
	}));
	if(size == 0.0f) return velocity;
	*/
	std::transform(velocity.begin(), velocity.end(), velocity.begin(), [=](float v) {
		return -v*s; // size;
	});
	return velocity;
}

std::vector<float> ParticleModify::GetAngularDrag(Particle &p, float s) {
	std::vector<float> result(p.rotation, p.rotation + 3);
	std::transform(result.begin(), result.end(), result.begin(), [=](float v) {
		return -v*s;
	});
	return result;
}

void ParticleModify::ChangeAngularVelocity(Particle &p, float x, float y, float z) {
	p.angularVelocity[0] += x;
	p.angularVelocity[1] += y;
	p.angularVelocity[2] += z;
}
void ParticleModify::AngularAccelerate(Particle &p, const std::vector<float> &acc, float t) {
	if(acc.size() != 3) return;
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


void ParticleModify::Reset(Particle &p) {
	p.alpha = p.alphaInit;
	p.size = p.sizeInit;
	std::memcpy(p.color, p.colorInit, 3 * sizeof(float));
}