#pragma once
#include "WPJson.h"
#include "WPMaterial.h" 
#include <vector>

namespace wallpaper
{
namespace wpscene
{

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
    std::vector<float> directions {1.0f, 0, 0};
	float distancemax;
	float distancemin;
	int32_t id;
	std::string name;
	float rate {1.0f};
};

class Particle {
public:
    bool FromJson(const nlohmann::json&);
    std::vector<Emitter> emitters;
    std::vector<Initializer> initializers;
    uint32_t maxcount;
    uint32_t starttime;
};

class WPParticleObject {
public:
    bool FromJson(const nlohmann::json&);
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
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Initializer, name, max, min);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Emitter, name, distancemax, distancemin, rate, directions);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Particle, emitters, initializers);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPParticleObject, name, origin, angles, scale, visible, particle, particleObj);
}
}



