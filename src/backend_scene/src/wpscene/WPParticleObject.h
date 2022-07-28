#pragma once
#include "WPJson.hpp"
#include "WPMaterial.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <array>
#include "Utils/BitFlags.hpp"
#include "Core/Literals.hpp"

namespace wallpaper
{
namespace fs
{
class VFS;
}
namespace wpscene
{

class ParticleControlpoint {
public:
    enum class FlagEnum
    {
        link_mouse = 0, // 1
        // this control point will follow the mouse cursor.
        worldspace = 1, // 2
        // the control point will always be at the same position in the world, independent from the
        // position of the particle system.
    };
    using EFlags = BitFlags<FlagEnum>;

    bool                 FromJson(const nlohmann::json&);
    EFlags               flags { 0 };
    i32                  id { -1 };
    std::array<float, 3> offset { 0, 0, 0 };
    // a static offset relative to the position of the particle system.
};

class ParticleRender {
public:
    bool        FromJson(const nlohmann::json&);
    std::string name;
    float       length { 0.05f };
    float       maxlength { 10.0f };
    float       subdivision { 3.0f };
};

class Initializer {
public:
    bool                 FromJson(const nlohmann::json&);
    std::array<float, 3> max { 0, 0, 0 };
    std::array<float, 3> min { 0, 0, 0 };
    std::string          name;
};

class Emitter {
public:
    enum class FlagEnum : uint32_t
    {
        one_per_frame = 1,
    };
    using EFlags = BitFlags<FlagEnum>;

public:
    bool                   FromJson(const nlohmann::json&);
    std::array<float, 3>   directions { 1.0f, 1.0f, 0 };
    std::array<float, 3>   distancemax { 256.0f, 256.0f, 256.0f };
    std::array<float, 3>   distancemin { 0.0f, 0.0f, 0.0f };
    std::array<float, 3>   origin { 0, 0, 0 };
    std::array<int32_t, 3> sign { 0, 0, 0 };
    u32                    audioprocessingmode { 0 };
    i32                    controlpoint { 0 };
    int32_t                id;
    EFlags                 flags;
    std::string            name;
    float                  rate { 5.0f };
};

class Particle {
public:
    enum class FlagEnum
    {
        wordspace             = 0, // 1
        spritenoframeblending = 1, // 2
        perspective           = 2, // 4
    };
    using EFlags = BitFlags<FlagEnum>;

public:
    bool                              FromJson(const nlohmann::json&);
    std::vector<Emitter>              emitters;
    std::vector<nlohmann::json>       initializers;
    std::vector<nlohmann::json>       operators;
    std::vector<ParticleRender>       renderers;
    std::vector<ParticleControlpoint> controlpoints;
    std::string                       animationmode;
    float                             sequencemultiplier;
    uint32_t                          maxcount;
    uint32_t                          starttime;
    EFlags                            flags;
};

class ParticleInstanceoverride {
public:
    bool FromJosn(const nlohmann::json&);
    bool enabled { false };
    bool overColor { false };
    bool overColorn { false };

    float                alpha { 1.0f };
    float                count { 1.0f };
    float                lifetime { 1.0f };
    float                rate { 1.0f };
    float                speed { 1.0f };
    float                size { 1.0f };
    std::array<float, 3> color { 1.0f, 1.0f, 1.0f };
    std::array<float, 3> colorn { 1.0f, 1.0f, 1.0f };
};

class WPParticleObject {
public:
    bool                     FromJson(const nlohmann::json&, fs::VFS&);
    int32_t                  id;
    std::string              name;
    std::array<float, 3>     origin { 0.0f, 0.0f, 0.0f };
    std::array<float, 3>     scale { 1.0f, 1.0f, 1.0f };
    std::array<float, 3>     angles { 0.0f, 0.0f, 0.0f };
    std::array<float, 2>     parallaxDepth { 0.0f, 0.0f };
    bool                     visible { true };
    std::string              particle;
    std::string              render;
    WPMaterial               material;
    Particle                 particleObj;
    ParticleInstanceoverride instanceoverride;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Initializer, name, max, min);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Emitter, name, distancemax, distancemin, rate, directions);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Particle, initializers, operators, emitters);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPParticleObject, name, origin, angles, scale, visible, particle,
                                   particleObj);
} // namespace wpscene
} // namespace wallpaper
