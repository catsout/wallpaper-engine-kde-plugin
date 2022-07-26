#pragma once
#include "ParticleEmitter.h"
#include "Interface/IParticleRawGener.h"
#include "Utils/NoCopyMove.hpp"

#include <memory>

namespace wallpaper
{

enum class ParticleAnimationMode
{
    SEQUENCE,
    RANDOMONE
};

class ParticleSystem;

class ParticleSubSystem : NoCopy, NoMove {
public:
    ParticleSubSystem(ParticleSystem& p, std::shared_ptr<SceneMesh> sm, uint32_t maxcount,
                      float rate, ParticleRawGenSpecOp specOp);
    ~ParticleSubSystem();

    void Emitt();

    void AddEmitter(ParticleEmittOp&&);
    void AddInitializer(ParticleInitOp&&);
    void AddOperator(ParticleOperatorOp&&);

private:
    ParticleSystem&            parent;
    std::shared_ptr<SceneMesh> m_mesh;
    //	std::vector<std::unique_ptr<ParticleEmitter>> m_emiters;
    std::vector<ParticleEmittOp> m_emiters;

    std::vector<Particle>           m_particles;
    std::vector<ParticleInitOp>     m_initializers;
    std::vector<ParticleOperatorOp> m_operators;
    ParticleRawGenSpecOp            m_genSpecOp;
    uint32_t                        m_maxcount;
    float                           m_rate;
    double                          m_time;
};

class Scene;
class ParticleSystem : NoCopy, NoMove {
public:
    ParticleSystem(Scene& scene): scene(scene) {};
    ~ParticleSystem() = default;

    void Emitt();

    Scene& scene;

    std::vector<std::unique_ptr<ParticleSubSystem>> subsystems;
    std::unique_ptr<IParticleRawGener>              gener;
};
} // namespace wallpaper
