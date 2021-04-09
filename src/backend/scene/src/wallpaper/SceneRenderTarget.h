#pragma once
#include "SceneTexture.h"

namespace wallpaper
{
struct SceneBindRenderTarget {
    std::string name;
    bool copy {false};
    float scale {1.0f};
};

struct SceneRenderTarget {
    uint32_t width;
    uint32_t height;
    bool allowReuse {false};
    bool withDepth {false};
    SceneTextureSample sample {TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE,
                                TextureFilter::LINEAR, TextureFilter::LINEAR};
};
}