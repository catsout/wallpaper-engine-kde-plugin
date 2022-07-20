#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include "Type.hpp"
#include "SpriteAnimation.hpp"
#include "Scene/SceneTexture.h"
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{

union ImageExtra {
    int32_t val;
    char    str[125];
};

typedef std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> ImageDataPtr;

struct ImageData {
    uint32_t     width;
    uint32_t     height;
    uint32_t     size;
    ImageDataPtr data;
    ImageData() = default;
};

struct ImageHeader {
    // these two size is not for tex, just come from we
    // using Slot's size for tex
    uint16_t width;
    uint16_t height;
    uint16_t mapWidth;
    uint16_t mapHeight;

    bool mipmap_larger { false };
    bool mipmap_pow2 { false };

    ImageType     type { ImageType::UNKNOWN };
    TextureFormat format;
    uint32_t      count;
    bool          isSprite;
    TextureSample sample;

    SpriteAnimation spriteAnim;
    // for specific property
    std::unordered_map<std::string, ImageExtra> extraHeader;
};

// slot is one singal image
struct Image : NoCopy, NoMove {
    struct Slot {
        uint16_t               width;
        uint16_t               height;
        std::vector<ImageData> mipmaps;

        operator bool() { return width * height * mipmaps.size() > 0; }
    };
    ImageHeader       header;
    std::vector<Slot> slots;
    std::string       key;
};

} // namespace wallpaper
