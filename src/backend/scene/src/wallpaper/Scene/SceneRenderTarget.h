#pragma once
#include "SceneTexture.h"

namespace wallpaper
{

struct SceneRenderTarget {
    struct Bind {
        bool enable {false};
        std::string name;
        bool screen {false};
        float scale {1.0f};
    };

    uint16_t width;
    uint16_t height;
    bool allowReuse {false};
    bool withDepth {false};
    TextureSample sample {TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE,
                                TextureFilter::LINEAR, TextureFilter::LINEAR};
    Bind bind;
};
}