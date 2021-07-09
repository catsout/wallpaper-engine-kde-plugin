#include "ParticleSystem.h"
#include "Log.h"
#include "Scene.h"
#include "ParticleModify.h"
#include <algorithm>

using namespace wallpaper;

void updateIndexArray(std::size_t index, std::size_t count, SceneIndexArray& iarray) {
	std::vector<uint32_t> indexs;
	for(uint32_t i=0;i<count;i++) {
		uint32_t x = index*4 + i*4;
		std::vector<uint32_t> t {
			x, x+1, x+3,
			x+1, x+2, x+3
		};
		indexs.insert(indexs.end(), t.begin(), t.end());
	}
	iarray.Assign(index*6, &indexs[0], count*6);
}

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
		if(i == 0) {
			//LOG_INFO(std::to_string(p.velocity[1]));
		}
		i++;
	}

	m_mesh->SetDirty();
	auto& sv = m_mesh->GetVertexArray(0);
	auto& si = m_mesh->GetIndexArray(0);
	i = 0;
	for(const auto& p:m_particles) {
		sv.SetVertexs((i++)*4, 4, &(parent.gener->GenGLData(p, sv)[0]));
	}
	if(m_particles.size() > si.DataCount()/6) {
		updateIndexArray(0, m_particles.size(), si);
	}
}

void ParticleSystem::Emitt() {
	for(auto& el:subsystems) {
		el->Emitt();
	}
}