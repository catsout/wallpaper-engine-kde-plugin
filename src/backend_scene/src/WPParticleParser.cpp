#include "WPParticleParser.hpp"
#include "Particle/ParticleModify.h"
#include "Particle/ParticleSystem.h"
#include <random>
#include <memory>
#include <algorithm>
#include <cmath>
#include <functional>

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "Utils/Logging.h"
#include "Utils/Algorism.h"

using namespace wallpaper;
using namespace Eigen;
namespace PM = ParticleModify;

static std::uniform_real_distribution<float> ur(0.0f, 1.0f);

typedef const std::vector<float>& cFloats;
typedef std::vector<float>        Floats;
typedef std::function<float()>    RandomFn;

namespace
{

inline double GetRandomIn(double min, double max, double random) {
    return min + (max - min) * random;
}

inline void Color(Particle& p, double, RandomFn& rf, const std::array<float, 3> min,
                  const std::array<float, 3> max) {
    float                random = rf();
    std::array<float, 3> result;
    for (int32_t i = 0; i < 3; i++) {
        result[i] = GetRandomIn(min[i], max[i], random);
    }
    PM::InitColor(p, result[0], result[1], result[2]);
}

inline Vector3d GenRandomVec3(const RandomFn& rf, const std::array<float, 3>& min,
                              const std::array<float, 3>& max) {
    Vector3d result(3);
    for (int32_t i = 0; i < 3; i++) {
        result[i] = GetRandomIn(min[i], max[i], rf());
    }
    return result;
}

} // namespace

struct SingleRandom {
    float       min { 0.0f };
    float       max { 0.0f };
    float       exponent { 1.0f };
    static void ReadFromJson(const nlohmann::json& j, SingleRandom& r) {
        GET_JSON_NAME_VALUE_NOWARN(j, "min", r.min);
        GET_JSON_NAME_VALUE_NOWARN(j, "max", r.max);
    };
};
struct VecRandom {
    std::array<float, 3> min { 0.0f, 0.0f, 0.0f };
    std::array<float, 3> max { 0.0f, 0.0f, 0.0f };
    float                exponent { 1.0f };

    static void ReadFromJson(const nlohmann::json& j, VecRandom& r) {
        GET_JSON_NAME_VALUE_NOWARN(j, "min", r.min);
        GET_JSON_NAME_VALUE_NOWARN(j, "max", r.max);
    };
};
struct TurbulentRandom {
    float  scale { 1.0f };
    double timescale { 1.0f };
    float  offset { 0.0f };
    float  speedmin { 100.0f };
    float  speedmax { 250.0f };
    float  phasemin { 0.0f };
    float  phasemax { 0.1f };

    std::array<float, 3> forward { 0.0f, 1.0f, 0.0f }; // x y z
    std::array<float, 3> right { 0.0f, 0.0f, 1.0f };
    std::array<float, 3> up { 1.0f, 0.0f, 0.0f };

    static void ReadFromJson(const nlohmann::json& j, TurbulentRandom& r) {
        GET_JSON_NAME_VALUE_NOWARN(j, "scale", r.scale);
        GET_JSON_NAME_VALUE_NOWARN(j, "timescale", r.timescale);
        GET_JSON_NAME_VALUE_NOWARN(j, "offset", r.offset);
        GET_JSON_NAME_VALUE_NOWARN(j, "speedmin", r.speedmin);
        GET_JSON_NAME_VALUE_NOWARN(j, "speedmax", r.speedmax);
        GET_JSON_NAME_VALUE_NOWARN(j, "phasemin", r.phasemin);
        GET_JSON_NAME_VALUE_NOWARN(j, "phasemax", r.phasemax);
        GET_JSON_NAME_VALUE_NOWARN(j, "forward", r.forward);
        GET_JSON_NAME_VALUE_NOWARN(j, "right", r.right);
        GET_JSON_NAME_VALUE_NOWARN(j, "up", r.up);
    };
};
template<std::size_t N>
std::array<float, N> mapVertex(const std::array<float, N>& v, float (*oper)(float)) {
    std::array<float, N> result;
    std::transform(v.begin(), v.end(), result.begin(), oper);
    return result;
};

ParticleInitOp WPParticleParser::genParticleInitOp(const nlohmann::json& wpj, RandomFn rf) {
    using namespace std::placeholders;
    do {
        if (! wpj.contains("name")) break;
        std::string name;
        GET_JSON_NAME_VALUE(wpj, "name", name);

        if (name == "colorrandom") {
            VecRandom r;
            r.min = { 0.0f, 0.0f, 0.0f };
            r.max = { 255.0f, 255.0f, 255.0f };
            VecRandom::ReadFromJson(wpj, r);
            return std::bind(Color,
                             _1,
                             _2,
                             rf,
                             mapVertex(r.min,
                                       [](float x) {
                                           return x / 255.0f;
                                       }),
                             mapVertex(r.max, [](float x) {
                                 return x / 255.0f;
                             }));
        } else if (name == "lifetimerandom") {
            SingleRandom r = { 0.0f, 1.0f };
            SingleRandom::ReadFromJson(wpj, r);
            return [=](Particle& p, double) {
                PM::InitLifetime(p, GetRandomIn(r.min, r.max, rf()));
            };
        } else if (name == "sizerandom") {
            SingleRandom r = { 0.0f, 20.0f };
            SingleRandom::ReadFromJson(wpj, r);
            return [=](Particle& p, double) {
                PM::InitSize(p, GetRandomIn(r.min, r.max, rf()));
            };
        } else if (name == "alpharandom") {
            SingleRandom r = { 0.05f, 1.0f };
            SingleRandom::ReadFromJson(wpj, r);
            return [=](Particle& p, double) {
                PM::InitAlpha(p, GetRandomIn(r.min, r.max, rf()));
            };
        } else if (name == "velocityrandom") {
            VecRandom r;
            r.min[0] = r.min[1] = -32.0f;
            r.max[0] = r.max[1] = 32.0f;
            VecRandom::ReadFromJson(wpj, r);
            return [=](Particle& p, double) {
                auto result = GenRandomVec3(rf, r.min, r.max);
                PM::ChangeVelocity(p, result[0], result[1], result[2]);
            };
        } else if (name == "rotationrandom") {
            VecRandom r;
            r.max[2] = 2 * M_PI;
            VecRandom::ReadFromJson(wpj, r);
            return [=](Particle& p, double) {
                auto result = GenRandomVec3(rf, r.min, r.max);
                PM::ChangeRotation(p, result[0], result[1], result[2]);
            };
        } else if (name == "angularvelocityrandom") {
            VecRandom r;
            r.min[2] = -5.0f;
            r.max[2] = 5.0f;
            VecRandom::ReadFromJson(wpj, r);
            return [=](Particle& p, double) {
                auto result = GenRandomVec3(rf, r.min, r.max);
                PM::ChangeAngularVelocity(p, result[0], result[1], result[2]);
            };
        } else if (name == "turbulentvelocityrandom") {
            // to do
            TurbulentRandom r;
            TurbulentRandom::ReadFromJson(wpj, r);
            Vector3f forward(r.forward.data());
            Vector3f right(r.right.data());
            Vector3f pos = GenRandomVec3(rf, { 0, 0, 0 }, { 10.0f, 10.0f, 10.0f }).cast<float>();
            return [=](Particle& p, double duration) mutable {
                float speed = GetRandomIn(r.speedmin, r.speedmax, rf());
                if (duration > 10.0f) {
                    pos[0] += speed;
                    duration = 0.0f;
                }
                Vector3f result;
                do {
                    result = algorism::CurlNoise(pos.cast<double>()).cast<float>();
                    pos += result * 0.005f / r.timescale;
                    duration -= 0.01f;
                } while (duration > 0.01f);
                // limit direction
                {
                    double c     = result.dot(forward) / (result.norm() * forward.norm());
                    float  a     = std::acos(c) / M_PI;
                    float  scale = r.scale / 2.0f;
                    if (a > scale) {
                        auto axis = result.cross(forward).normalized();
                        result    = AngleAxisf((a - a * scale) * M_PI, axis) * result;
                    }
                }
                // offset
                result = AngleAxisf(r.offset, right) * result;
                result *= speed;
                PM::ChangeVelocity(p, result[0], result[1], result[2]);
            };
        }
    } while (false);
    return [](Particle&, double) {
    };
}

ParticleInitOp WPParticleParser::genOverrideInitOp(const wpscene::ParticleInstanceoverride& over) {
    return [=](Particle& p, double) {
        PM::MutiplyInitLifeTime(p, over.lifetime);
        PM::MutiplyInitAlpha(p, over.alpha);
        PM::MutiplyInitSize(p, over.size);
        PM::MutiplyVelocity(p, over.speed);
        if (over.overColor) {
            PM::InitColor(
                p, over.color[0] / 255.0f, over.color[1] / 255.0f, over.color[2] / 255.0f);
        } else if (over.overColorn) {
            PM::MutiplyInitColor(p, over.colorn[0], over.colorn[1], over.colorn[2]);
        }
    };
}
double FadeValueChange(float life, float start, float end, float startValue,
                       float endValue) noexcept {
    if (life <= start)
        return startValue;
    else if (life > end)
        return endValue;
    else {
        double pass = (life - start) / (end - start);
        return algorism::lerp(pass, startValue, endValue);
    }
}

struct ValueChange {
    float starttime { 0 };
    float endtime { 1.0f };
    float startvalue { 1.0f };
    float endvalue { 0.0f };

    static auto ReadFromJson(const nlohmann::json& j) {
        ValueChange v;
        GET_JSON_NAME_VALUE_NOWARN(j, "starttime", v.starttime);
        GET_JSON_NAME_VALUE_NOWARN(j, "endtime", v.endtime);
        GET_JSON_NAME_VALUE_NOWARN(j, "startvalue", v.startvalue);
        GET_JSON_NAME_VALUE_NOWARN(j, "endvalue", v.endvalue);
        return v;
    }
};
double FadeValueChange(float life, const ValueChange& v) noexcept {
    return FadeValueChange(life, v.starttime, v.endtime, v.startvalue, v.endvalue);
}

struct VecChange {
    float                starttime { 0 };
    float                endtime { 1.0f };
    std::array<float, 3> startvalue { 0.0f, 0.0f, 0.0f };
    std::array<float, 3> endvalue { 0.0f, 0.0f, 0.0f };

    static auto ReadFromJson(const nlohmann::json& j) {
        VecChange v;
        GET_JSON_NAME_VALUE_NOWARN(j, "starttime", v.starttime);
        GET_JSON_NAME_VALUE_NOWARN(j, "endtime", v.endtime);
        GET_JSON_NAME_VALUE_NOWARN(j, "startvalue", v.startvalue);
        GET_JSON_NAME_VALUE_NOWARN(j, "endvalue", v.endvalue);
        return v;
    }
};

struct FrequencyValue {
    std::array<float, 3> mask { 1.0f, 1.0f, 0.0f };

    float frequencymin { 0.0f };
    float frequencymax { 10.0f };
    float scalemin { 0.0f };
    float scalemax { 1.0f };
    float phasemin { 0.0f };
    float phasemax { static_cast<float>(2 * M_PI) };

    struct StorageRandom {
        bool  reset { true };
        float frequency { 0.0f };
        float scale { 1.0f };
        float phase { 0.0f };
    };

    std::vector<StorageRandom> storage;

    static auto ReadFromJson(const nlohmann::json& j, std::string_view name) {
        FrequencyValue v;
        if (name == "oscillatesize") {
            v.scalemin = 0.8f;
            v.scalemax = 1.2f;
        } else if (name == "oscillateposition") {
            v.frequencymax = 5.0f;
        }
        GET_JSON_NAME_VALUE_NOWARN(j, "frequencymin", v.frequencymin);
        GET_JSON_NAME_VALUE_NOWARN(j, "frequencymax", v.frequencymax);
        if (v.frequencymax == 0.0f) v.frequencymax = v.frequencymin;
        GET_JSON_NAME_VALUE_NOWARN(j, "scalemin", v.scalemin);
        GET_JSON_NAME_VALUE_NOWARN(j, "scalemax", v.scalemax);
        GET_JSON_NAME_VALUE_NOWARN(j, "phasemin", v.phasemin);
        GET_JSON_NAME_VALUE_NOWARN(j, "phasemax", v.phasemax);
        GET_JSON_NAME_VALUE_NOWARN(j, "mask", v.mask);
        return v;
    };
    inline void CheckAndResize(size_t s) {
        if (storage.size() < s) storage.resize(2 * s, StorageRandom {});
    }
    inline void GenFrequency(Particle& p, uint32_t index, RandomFn& rf) {
        auto& st = storage.at(index);
        if (! PM::LifetimeOk(p)) st.reset = true;
        if (st.reset) {
            st.frequency = GetRandomIn(frequencymin, frequencymax, rf());
            st.scale     = GetRandomIn(scalemin, scalemax, rf());
            st.phase     = GetRandomIn(phasemin, phasemax + 2.0f * M_PI, rf());
            st.reset     = false;
        }
    }
    inline double GetScale(uint32_t index, double time) {
        const auto& st = storage.at(index);
        double      f  = st.frequency / (2.0f * M_PI);
        double      w  = 2.0f * M_PI * f;
        return algorism::lerp((std::cos(w * time + st.phase) + 1.0f) * 0.5f, scalemin, scalemax);
    }
    inline double GetMove(uint32_t index, double time, double timePass) {
        const auto& st = storage.at(index);
        double      f  = st.frequency / (2.0f * M_PI);
        double      w  = 2.0f * M_PI * f;
        return -1.0f * st.scale * w * std::sin(w * time + st.phase) * timePass;
    }
};

struct Turbulence {
    float phasemin { 0 }; // the minimum time offset of the noise field for a particle.
    float phasemax { 0 };
    float speedmin { 500.0f };
    float speedmax { 1000.0f };
    float timescale { 20.0f }; // how fast the noise field changes shape

    std::array<int32_t, 3> mask { 1, 1, 0 };

    static auto ReadFromJson(const nlohmann::json& j) {
        Turbulence v;
        GET_JSON_NAME_VALUE_NOWARN(j, "phasemin", v.phasemin);
        GET_JSON_NAME_VALUE_NOWARN(j, "phasemax", v.phasemax);
        GET_JSON_NAME_VALUE_NOWARN(j, "speedmin", v.speedmin);
        GET_JSON_NAME_VALUE_NOWARN(j, "speedmax", v.speedmax);
        GET_JSON_NAME_VALUE_NOWARN(j, "timescale", v.timescale);
        GET_JSON_NAME_VALUE_NOWARN(j, "mask", v.mask);
        return v;
    };
};

ParticleOperatorOp
WPParticleParser::genParticleOperatorOp(const nlohmann::json& wpj, RandomFn rf,
                                        const wpscene::ParticleInstanceoverride& over) {
    do {
        if (! wpj.contains("name")) break;
        std::string name;
        GET_JSON_NAME_VALUE(wpj, "name", name);
        if (name == "movement") {
            float drag { 0.0f };
            auto  speed = over.speed;

            std::array<float, 3> gravity { 0, 0, 0 };
            GET_JSON_NAME_VALUE_NOWARN(wpj, "drag", drag);
            GET_JSON_NAME_VALUE_NOWARN(wpj, "gravity", gravity);
            Vector3d vecG = Vector3f(gravity.data()).cast<double>();
            return [vecG = std::move(vecG), drag, speed](const ParticleInfo& info) {
                for (auto& p : info.particles) {
                    Vector3d acc =
                        algorism::DragForce(PM::GetVelocity(p).cast<double>(), drag) + vecG;
                    PM::Accelerate(p, speed * acc, info.time_pass);
                    PM::MoveByTime(p, info.time_pass);
                }
            };
        } else if (name == "angularmovement") {
            float                drag { 0.0f };
            std::array<float, 3> force { 0, 0, 0 };
            GET_JSON_NAME_VALUE_NOWARN(wpj, "drag", drag);
            GET_JSON_NAME_VALUE_NOWARN(wpj, "force", force);
            Vector3d vecF = Vector3f(force.data()).cast<double>();
            return [=](const ParticleInfo& info) {
                for (auto& p : info.particles) {
                    Vector3d acc =
                        algorism::DragForce(PM::GetAngular(p).cast<double>(), drag) + vecF;
                    PM::AngularAccelerate(p, acc, info.time_pass);
                    PM::RotateByTime(p, info.time_pass);
                }
            };
        } else if (name == "sizechange") {
            auto vc        = ValueChange::ReadFromJson(wpj);
            auto size_over = over.size;
            return [vc, size_over](const ParticleInfo& info) {
                for (auto& p : info.particles)
                    PM::MutiplySize(p, size_over * FadeValueChange(PM::LifetimePos(p), vc));
            };

        } else if (name == "alphafade") {
            float fadeintime { 0.5f }, fadeouttime { 0.5f };
            GET_JSON_NAME_VALUE_NOWARN(wpj, "fadeintime", fadeintime);
            GET_JSON_NAME_VALUE_NOWARN(wpj, "fadeouttime", fadeouttime);
            return [fadeintime, fadeouttime](const ParticleInfo& info) {
                for (auto& p : info.particles) {
                    auto life = PM::LifetimePos(p);
                    if (life <= fadeintime)
                        PM::MutiplyAlpha(p, FadeValueChange(life, 0, fadeintime, 0, 1.0f));
                    else if (life > fadeouttime)
                        PM::MutiplyAlpha(p,
                                         1.0f - FadeValueChange(life, fadeouttime, 1.0f, 0, 1.0f));
                }
            };
        } else if (name == "alphachange") {
            auto vc = ValueChange::ReadFromJson(wpj);
            return [vc](const ParticleInfo& info) {
                for (auto& p : info.particles) {
                    PM::MutiplyAlpha(p, FadeValueChange(PM::LifetimePos(p), vc));
                }
            };
        } else if (name == "colorchange") {
            auto vc = VecChange::ReadFromJson(wpj);
            return [vc](const ParticleInfo& info) {
                for (auto& p : info.particles) {
                    auto     life = PM::LifetimePos(p);
                    Vector3f result;
                    for (uint i = 0; i < 3; i++)
                        result[i] = FadeValueChange(
                            life, vc.starttime, vc.endtime, vc.startvalue[i], vc.endvalue[i]);
                    PM::MutiplyColor(p, result[0], result[1], result[2]);
                }
            };
        } else if (name == "oscillatealpha") {
            FrequencyValue fv = FrequencyValue::ReadFromJson(wpj, name);
            return [fv, rf](const ParticleInfo& info) mutable {
                fv.CheckAndResize(info.particles.size());
                for (uint i = 0; i < info.particles.size(); i++) {
                    auto& p = info.particles[i];
                    fv.GenFrequency(p, i, rf);
                    PM::MutiplyAlpha(p, fv.GetScale(i, PM::LifetimePassed(p)));
                }
            };
        } else if (name == "oscillatesize") {
            FrequencyValue fv = FrequencyValue::ReadFromJson(wpj, name);
            return [fv, rf](const ParticleInfo& info) mutable {
                fv.CheckAndResize(info.particles.size());
                for (uint i = 0; i < info.particles.size(); i++) {
                    auto& p = info.particles[i];
                    fv.GenFrequency(p, i, rf);
                    PM::MutiplySize(p, fv.GetScale(i, PM::LifetimePassed(p)));
                }
            };

        } else if (name == "oscillateposition") {
            std::vector<Vector3f>         lastMove;
            FrequencyValue                fvx = FrequencyValue::ReadFromJson(wpj, name);
            std::array<FrequencyValue, 3> fxp = { fvx, fvx, fvx };
            return [=](const ParticleInfo& info) mutable {
                for (auto& f : fxp) f.CheckAndResize(info.particles.size());
                for (uint i = 0; i < info.particles.size(); i++) {
                    auto&    p = info.particles[i];
                    Vector3d del { Vector3d::Zero() };
                    auto     time = PM::LifetimePassed(p);
                    for (uint d = 0; d < 3; d++) {
                        if (fxp[0].mask[d] < 0.01) continue;
                        fxp[d].GenFrequency(p, i, rf);
                        del[d] = fxp[d].GetMove(i, time, info.time_pass);
                    }

                    PM::Move(p, del);
                }
            };
        } else if (name == "turbulence") {
            Turbulence tur = Turbulence::ReadFromJson(wpj);
            return [=](const ParticleInfo& info) {
                for (auto& p : info.particles) {
                    double   speed  = GetRandomIn(tur.speedmin, tur.speedmax, rf());
                    double   phase  = GetRandomIn(tur.phasemin, tur.phasemax, rf());
                    Vector3d pos    = PM::GetPos(p).cast<double>();
                    Vector3d result = speed * algorism::CurlNoise(pos / tur.timescale / 2.0f);
                    result[0] *= tur.mask[0];
                    result[1] *= tur.mask[1];
                    result[2] *= tur.mask[2];
                    PM::Accelerate(p, result, info.time_pass);
                }
            };
        }
    } while (false);
    return [](const ParticleInfo&) {
    };
}

ParticleEmittOp WPParticleParser::genParticleEmittOp(const wpscene::Emitter& wpe, RandomFn rf,
                                                     bool sort) {
    if (wpe.name == "boxrandom") {
        ParticleBoxEmitterArgs box;
        box.emitSpeed     = wpe.rate;
        box.minDistance   = wpe.distancemin;
        box.maxDistance   = wpe.distancemax;
        box.directions    = wpe.directions;
        box.orgin         = wpe.origin;
        box.randomFn      = rf;
        box.one_per_frame = wpe.flags[wpscene::Emitter::FlagEnum::one_per_frame];
        box.sort          = sort;
        return ParticleBoxEmitterArgs::MakeEmittOp(box);
    } else if (wpe.name == "sphererandom") {
        ParticleSphereEmitterArgs sphere;
        sphere.emitSpeed     = wpe.rate;
        sphere.minDistance   = wpe.distancemin[0];
        sphere.maxDistance   = wpe.distancemax[0];
        sphere.directions    = wpe.directions;
        sphere.orgin         = wpe.origin;
        sphere.sign          = wpe.sign;
        sphere.randomFn      = rf;
        sphere.one_per_frame = wpe.flags[wpscene::Emitter::FlagEnum::one_per_frame];
        sphere.sort          = sort;
        return ParticleSphereEmitterArgs::MakeEmittOp(sphere);
    } else
        return [](std::vector<Particle>&, std::vector<ParticleInitOp>&, uint32_t, float) {
        };
}
