#pragma once
#include "WPJson.h"
#include "WPMaterial.h" 
#include <vector>

namespace wallpaper
{
namespace wpscene
{

class WPEffectFbo {
public:
    bool FromJson(const nlohmann::json&);
    std::string name;
    std::string format;
    int32_t scale {1};
};

class WPImageEffect {
public:
    bool FromJson(const nlohmann::json&);
    bool FromFileJson(const nlohmann::json&);
    int32_t id;
    std::string name;
    bool visible {true};
    int32_t version;
    std::vector<WPMaterial> materials;
    std::vector<WPMaterialPass> passes;
    std::vector<WPEffectFbo> fbos;
};

class WPImageObject {
public:
    bool FromJson(const nlohmann::json&);
    int32_t id;
    std::string name;
    std::vector<float> origin {0.0f, 0.0f, 0.0f};
    std::vector<float> scale {1.0f, 1.0f, 1.0f};
    std::vector<float> angles {0.0f, 0.0f, 0.0f};
    std::vector<float> size {2.0f, 2.0f};
    std::vector<float> parallaxDepth {0.0f, 0.0f};
    std::vector<float> color {1.0f, 1.0f, 1.0f};
    int32_t colorBlendMode {0};
    float alpha {1.0f};
    float brightness {1.0f};
    bool fullscreen {false};
    bool visible {true};
    std::string image;
    WPMaterial material;    
    std::vector<WPImageEffect> effects;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPEffectFbo, name, scale);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPImageEffect, name, visible, passes, fbos, materials);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPImageObject, name, origin, angles, scale, size, visible, material, effects);

}
}