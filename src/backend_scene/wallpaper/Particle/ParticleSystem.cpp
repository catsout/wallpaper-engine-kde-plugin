#include "ParticleSystem.h"
#include "Scene/Scene.h"
#include "ParticleModify.h"
#include <algorithm>

using namespace wallpaper;

void ParticleSubSystem::AddEmitter(ParticleEmittOp&& em) { m_emiters.emplace_back(em); }

void ParticleSubSystem::AddInitializer(ParticleInitOp&& ini) { m_initializers.emplace_back(ini); }

void ParticleSubSystem::AddOperator(ParticleOperatorOp&& op) { m_operators.emplace_back(op); }

void ParticleSubSystem::Emitt() {
    double frameTime    = parent.scene.frameTime;
    double particleTime = frameTime * m_rate;
    m_time += particleTime;

    for (auto& emittOp : m_emiters) {
        emittOp(m_particles, m_initializers, m_maxcount, particleTime);
    }

    uint32_t     i = 0;
    ParticleInfo info { .particles = { m_particles.data(), m_particles.size() },
                        .time      = m_time,
                        .time_pass = particleTime };

    for (auto& p : info.particles) {
        if (! ParticleModify::LifetimeOk(p)) {
            i++;
            continue;
        }
        ParticleModify::Reset(p);
        ParticleModify::ChangeLifetime(p, -particleTime);
    }

    std::for_each(m_operators.begin(), m_operators.end(), [&info](ParticleOperatorOp& op) {
        op(info);
    });

    m_mesh->SetDirty();
    parent.gener->GenGLData(m_particles, *m_mesh, m_genSpecOp);
}

void ParticleSystem::Emitt() {
    for (auto& el : subsystems) {
        el->Emitt();
    }
}