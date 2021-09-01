#include "ParticleSystem.h"
#include "Log.h"
#include "Scene/Scene.h"
#include "ParticleModify.h"
#include <algorithm>

using namespace wallpaper;



void ParticleSubSystem::AddEmitter(ParticleEmittOp&& em) {
	m_emiters.emplace_back(em);
}


void ParticleSubSystem::AddInitializer(ParticleInitOp&& ini) {
	m_initializers.emplace_back(ini);
}

void ParticleSubSystem::AddOperator(ParticleOperatorOp&& op) {
	m_operators.emplace_back(op);
}

void ParticleSubSystem::Emitt() {
	auto frameTime = parent.scene.frameTime;
	double particleTime = frameTime * m_rate;
	for(auto& emittOp:m_emiters) {
		emittOp(m_particles, m_initializers, m_maxcount, particleTime);	
	}

	uint32_t i = 0;
	for(auto& p:m_particles) {
		if(!ParticleModify::LifetimeOk(p)) { i++;continue; }
		ParticleModify::Reset(p);
		ParticleModify::ChangeLifetime(p, -particleTime);
		auto lifetimePos = ParticleModify::LifetimePos(p);
		std::for_each(m_operators.begin(), m_operators.end(), [&](ParticleOperatorOp& op) {
			op(p, i, lifetimePos, particleTime);
		});
		//if(i == 0) {}
		i++;
	}

	m_mesh->SetDirty();
	parent.gener->GenGLData(m_particles, *m_mesh, m_genSpecOp);
}

void ParticleSystem::Emitt() {
	for(auto& el:subsystems) {
		el->Emitt();
	}
}