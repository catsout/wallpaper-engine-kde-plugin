#pragma once
#include "WPJson.h"
#include "WPMaterial.h" 
#include <vector>

namespace wallpaper
{
namespace fs { class VFS; }
namespace wpscene
{

class ParticleRender {
public:
    bool FromJson(const nlohmann::json&);
    std::string name;
    float length {0.05f};
    float maxlength {10.0f};
};

class Initializer {
public:
    bool FromJson(const nlohmann::json&);
    std::vector<float> max {0};
    std::vector<float> min {0};
    std::string name;
};

class Emitter {
public:
    bool FromJson(const nlohmann::json&);
    std::vector<float> directions {1.0f, 1.0f, 0};
	std::vector<float> distancemax {256.0f};
	std::vector<float> distancemin {0.0f};
    std::vector<float> origin {0, 0, 0};
    std::vector<int32_t> sign {0, 0, 0};
	int32_t id;
	std::string name;
	float rate {5.0f};
};

struct ParticleFlag {
    bool wordspace {false};             // 1
    bool spritenoframeblending {false}; // 2
    bool perspective {false};           // 4
};

class Particle {
public:
    bool FromJson(const nlohmann::json&);
    std::vector<Emitter> emitters;
    std::vector<nlohmann::json> initializers;
    std::vector<nlohmann::json> operators;
    std::vector<ParticleRender> renderers;
    std::string animationmode;
    float sequencemultiplier;
    uint32_t maxcount;
    uint32_t starttime;
    ParticleFlag flags;
};

class ParticleInstanceoverride {
public:
    bool FromJosn(const nlohmann::json&);
    bool enabled {false};
    bool overColor {false};
    bool overColorn {false};
    
    float alpha {1.0f};
    float count {1.0f};
    float lifetime {1.0f};
    float rate {1.0f};
    float speed {1.0f};
    float size {1.0f};
    std::vector<float> color {1.0f, 1.0f, 1.0f};
    std::vector<float> colorn {1.0f, 1.0f, 1.0f};
};

class WPParticleObject {
public:
    bool FromJson(const nlohmann::json&, fs::VFS&);
    int32_t id;
    std::string name;
    std::vector<float> origin {0.0f, 0.0f, 0.0f};
    std::vector<float> scale {1.0f, 1.0f, 1.0f};
    std::vector<float> angles {0.0f, 0.0f, 0.0f};
    std::vector<float> parallaxDepth {0.0f, 0.0f};
    bool visible {true};
    std::string particle;
    std::string render;
    WPMaterial material;
    Particle particleObj;
    ParticleInstanceoverride instanceoverride;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Initializer, name, max, min);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Emitter, name, distancemax, distancemin, rate, directions);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ParticleFlag, wordspace, perspective);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Particle, flags, initializers, operators, emitters);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPParticleObject, name, origin, angles, scale, visible, particle, particleObj);
}
}



