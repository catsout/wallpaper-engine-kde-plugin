#pragma once
#include <cstdint>
#include <unordered_map>
#include <cstdint>
#include "WPJson.h"

namespace wallpaper
{

namespace wpscene
{

class Orthogonalprojection {
public:
    bool FromJson(const nlohmann::json&);
    int32_t width;
    int32_t height;
};

class WPSceneCamera {
public:
    bool FromJson(const nlohmann::json&);
    std::vector<float> center {0.0f, 0.0f, 0.0f};
    std::vector<float> eye {0.0f, 0.0f, 1.0f};
    std::vector<float> up {0.0f, 1.0f, 0.0f};
};

class WPSceneGeneral {
public:
    bool FromJson(const nlohmann::json&);
    std::vector<float> clearcolor {0.0f, 0.0f, 0.0f};
    bool cameraparallax {false};
    float cameraparallaxamount;
    float cameraparallaxdelay;
    float cameraparallaxmouseinfluence;
    bool isOrtho {true};
    Orthogonalprojection orthogonalprojection {1920, 1080};
    float zoom {1.0f};
};

class WPScene {
public:
    bool FromJson(const nlohmann::json&);
    WPSceneCamera camera;
    WPSceneGeneral general;
};    

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Orthogonalprojection, width, height);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPSceneCamera, center, eye, up);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPSceneGeneral, clearcolor, orthogonalprojection, zoom);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WPScene, camera, general);
}
} 