#include "ParticleEmitter.h"
#include "ParticleModify.h"
#include "Utils/Algorism.h"
#include "Utils/Logging.h"
#include <random>
#include <array>
#include <tuple>

using namespace wallpaper;

typedef std::function<Particle()> GenParticleOp;
typedef std::function<Particle()> SpwanOp;

namespace
{
inline float GetRandomIn(float min, float max, float random) { return min + (max - min) * random; }

inline std::tuple<uint32_t, bool> FindLastParticle(std::span<const Particle> ps, uint32_t last) {
    for (uint32_t i = last; i < ps.size(); i++) {
        if (! ParticleModify::LifetimeOk(ps[i])) return { i, true };
    }
    return { 0, false };
}

inline uint32_t GetEmitNum(double& timer, float speed) {
    double emitDur = 1.0f / speed;
    if (emitDur > timer) return 0;
    uint32_t num = timer / emitDur;
    while (emitDur < timer) timer -= emitDur;
    if (timer < 0) timer = 0;
    return num;
}
inline uint32_t Emitt(std::vector<Particle>& particles, uint32_t num, uint32_t maxcount, bool sort,
                      SpwanOp Spwan) {
    uint32_t lastPartcle = 0;
    bool     has_dead    = true;
    uint32_t i           = 0;

    for (i = 0; i < num; i++) {
        if (has_dead) {
            auto [r1, r2] = FindLastParticle(particles, lastPartcle);
            lastPartcle   = r1;
            has_dead      = r2;
        }
        if (has_dead) {
            particles[lastPartcle] = Spwan();

        } else {
            if (maxcount == particles.size()) break;
            particles.push_back(Spwan());
        }
    }

    if (sort) {
        // old << new << dead
        std::stable_sort(particles.begin(), particles.end(), [](const auto& a, const auto& b) {
            bool l_a = ParticleModify::LifetimeOk(a);
            bool l_b = ParticleModify::LifetimeOk(b);

            return (l_a && ! l_b) ||
                   (l_a && l_b && ! ParticleModify::IsNew(a) && ParticleModify::IsNew(b));
        });
    }

    return i + 1;
}

inline Particle Spwan(GenParticleOp gen, std::vector<ParticleInitOp>& inis, double duration) {
    auto particle = gen();
    for (auto& el : inis) el(particle, duration);
    return particle;
}
} // namespace

ParticleEmittOp ParticleBoxEmitterArgs::MakeEmittOp(ParticleBoxEmitterArgs a) {
    double timer { 0.0f };
    return [a, timer](std::vector<Particle>&       ps,
                      std::vector<ParticleInitOp>& inis,
                      uint32_t                     maxcount,
                      double                       timepass) mutable {
        timer += timepass;
        auto GenBox = [&]() {
            std::array<float, 3> pos;
            for (int32_t i = 0; i < 3; i++)
                pos[i] =
                    GetRandomIn(a.minDistance[i], a.maxDistance[i], (a.randomFn() - 0.5f) * 2.0f);
            auto p = Particle();
            ParticleModify::MoveTo(p, pos[0], pos[1], pos[2]);
            ParticleModify::MoveMultiply(p, a.directions[0], a.directions[1], a.directions[2]);
            ParticleModify::Move(p, a.orgin[0], a.orgin[1], a.orgin[2]);
            return p;
        };
        uint32_t emit_num = GetEmitNum(timer, a.emitSpeed);
        emit_num          = a.one_per_frame ? 1 : emit_num;
        Emitt(ps, emit_num, maxcount, a.sort, [&]() {
            return Spwan(GenBox, inis, 1.0f / a.emitSpeed);
        });
    };
}

ParticleEmittOp ParticleSphereEmitterArgs::MakeEmittOp(ParticleSphereEmitterArgs a) {
    using namespace Eigen;
    double timer { 0.0f };
    return [a, timer](std::vector<Particle>&       ps,
                      std::vector<ParticleInitOp>& inis,
                      uint32_t                     maxcount,
                      double                       timepass) mutable {
        timer += timepass;
        auto GenSphere = [&]() {
            auto                  p  = Particle();
            float                 r  = GetRandomIn(a.minDistance, a.maxDistance, a.randomFn());
            std::array<double, 3> sp = algorism::GenSphere(a.randomFn);
            ParticleModify::MoveTo(p, sp[0], sp[1], sp[2]);
            {
                ParticleModify::SphereDirectOffset(p,
                                                   Vector3d::UnitX(),
                                                   std::asin(p.position[0]) -
                                                       std::asin(p.position[0] * a.directions[0]));
                ParticleModify::SphereDirectOffset(p,
                                                   Vector3d::UnitY(),
                                                   std::asin(p.position[1]) -
                                                       std::asin(p.position[1] * a.directions[1]));
                ParticleModify::SphereDirectOffset(p,
                                                   Vector3d::UnitZ(),
                                                   std::asin(p.position[2]) -
                                                       std::asin(p.position[2] * a.directions[2]));
            }
            ParticleModify::MoveMultiply(p, r, r, r);
            ParticleModify::MoveApplySign(p, a.sign[0], a.sign[1], a.sign[2]);
            ParticleModify::Move(p, a.orgin[0], a.orgin[1], a.orgin[2]);
            return p;
        };
        uint32_t emit_num = GetEmitNum(timer, a.emitSpeed);
        emit_num          = a.one_per_frame ? 1 : emit_num;
        Emitt(ps, emit_num, maxcount, a.sort, [&]() {
            return Spwan(GenSphere, inis, 1.0f / a.emitSpeed);
        });
    };
}
