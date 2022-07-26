#pragma once
#include "SceneTexture.h"
#include "Core/Literals.hpp"

namespace wallpaper
{

struct SceneRenderTarget {
    struct Bind {
        bool        enable { false };
        std::string name {};
        bool        screen { false };
        double      scale { 1.0 };
    };

    i32           width;
    i32           height;
    bool          allowReuse { false };
    bool          withDepth { false };
    bool          has_mipmap { false };
    uint          mipmap_level { 1 };
    TextureSample sample { TextureWrap::CLAMP_TO_EDGE,
                           TextureWrap::CLAMP_TO_EDGE,
                           TextureFilter::LINEAR,
                           TextureFilter::LINEAR };
    Bind          bind {};
};
} // namespace wallpaper
