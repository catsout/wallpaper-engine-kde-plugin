#pragma once
#include "ParticleEmitter.h"
#include "SceneMesh.h"
#include "Interface/IParticleRawGener.h"

#include <memory>

namespace wallpaper {

class ParticleSystem;

class ParticleSubSystem {
public:
	ParticleSubSystem(ParticleSystem& p, std::shared_ptr<SceneMesh> sm):parent(p),m_mesh(sm) {};
	void Emitt();

	void AddEmitter(std::unique_ptr<ParticleEmitter>);
	void AddInitializer(std::shared_ptr<IParticleInitializer>);
private:
	ParticleSystem& parent;
	std::shared_ptr<SceneMesh> m_mesh;
	std::vector<std::unique_ptr<ParticleEmitter>> m_emiters;

	std::vector<Particle> m_particles;
	std::vector<std::shared_ptr<IParticleInitializer>> m_initializers;
};

class Scene;
class ParticleSystem {
public:
	ParticleSystem(Scene& scene):scene(scene) {};
	~ParticleSystem() = default;

	void Emitt();

	Scene& scene;

	std::vector<std::unique_ptr<ParticleSubSystem>> subsystems;
	std::unique_ptr<IParticleRawGener> gener;
};
}