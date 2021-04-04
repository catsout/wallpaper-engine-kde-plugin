#pragma once
#include "SceneTexture.h"

namespace wallpaper
{
struct SceneRenderTarget {
    uint32_t width;
    uint32_t height;
    uint32_t index;
    bool allowReuse {false};
    bool withDepth {false};
    SceneTextureSample sample {TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE,
                                TextureFilter::LINEAR, TextureFilter::LINEAR};
};
}