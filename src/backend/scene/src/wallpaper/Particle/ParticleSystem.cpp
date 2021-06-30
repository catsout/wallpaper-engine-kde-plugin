#include "ParticleSystem.h"
#include "Log.h"
#include "Scene.h"

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

void ParticleSubSystem::AddEmitter(std::unique_ptr<ParticleEmitter> em) {
	m_emiters.emplace_back(std::move(em));
}

void ParticleSubSystem::AddInitializer(std::shared_ptr<IParticleInitializer> ini) {
	m_initializers.emplace_back(ini);
}

void ParticleSubSystem::Emitt() {
	for(auto& el:m_emiters) {
		el->TimePass(parent.scene.frameTime);
		int32_t x = el->Emmit(m_particles, m_initializers);
	}

	for(auto& p:m_particles) {
		p.lifetime -= parent.scene.frameTime;
	}

	m_mesh->SetDirty();
	auto& sv = m_mesh->GetVertexArray(0);
	auto& si = m_mesh->GetIndexArray(0);
	uint32_t i = 0;
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