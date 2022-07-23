#include "ParticleSystem.h"
#include "Scene/Scene.h"
#include "ParticleModify.h"
#include "Scene/SceneMesh.h"

#include <algorithm>

using namespace wallpaper;

ParticleSubSystem::ParticleSubSystem(ParticleSystem& p, std::shared_ptr<SceneMesh> sm,
                                     uint32_t maxcount, float rate, ParticleRawGenSpecOp specOp)
    : parent(p), m_mesh(sm), m_maxcount(maxcount), m_rate(rate), m_genSpecOp(specOp), m_time(0) {};

ParticleSubSystem::~ParticleSubSystem() = default;

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
        ParticleModify::MarkOld(p);
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
