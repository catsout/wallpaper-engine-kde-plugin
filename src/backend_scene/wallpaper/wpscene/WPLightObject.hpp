#pragma once
#include "WPJson.hpp"
#include "WPMaterial.h"
#include <vector>
#include "WPPuppet.hpp"

namespace wallpaper
{
namespace fs
{
class VFS;
}

namespace wpscene
{

class WPLightObject {
public:
    bool                 FromJson(const nlohmann::json&);
    int32_t              id { 0 };
    std::string          name;
    std::array<float, 3> origin { 0.0f, 0.0f, 0.0f };
    std::array<float, 3> scale { 1.0f, 1.0f, 1.0f };
    std::array<float, 3> angles { 0.0f, 0.0f, 0.0f };
    std::array<float, 2> parallaxDepth { 0.0f, 0.0f };
    std::array<float, 3> color { 1.0f, 1.0f, 1.0f };
    std::string          light;
    float                radius { 1000.0f };
    float                intensity { 1.0f };
    bool                 visible { true };
};

} // namespace wpscene
} // namespace wallpaper
  /*
          {
              "angles" : "0.00000 0.00000 0.00000",
              "color" : "1.00000 0.95686 0.87451",
              "id" : 237,
              "intensity" : 0.5,
              "light" : "point",
              "locktransforms" : false,
              "name" : "",
              "origin" : "611.30676 302.13736 2000.00000",
              "parallaxDepth" : "0.00000 0.00000",
              "radius" : 3000.0,
              "scale" : "1.00000 1.00000 1.00000",
              "visible" : true
          },
  */