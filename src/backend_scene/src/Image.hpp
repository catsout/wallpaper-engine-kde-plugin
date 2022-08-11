#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include "Core/Literals.hpp"
#include "Type.hpp"
#include "SpriteAnimation.hpp"
#include "Scene/SceneTexture.h"
#include "Core/NoCopyMove.hpp"

namespace wallpaper
{

union ImageExtra {
    int32_t val;
    char    str[125];
};

typedef std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> ImageDataPtr;

struct ImageData {
    i32          width;
    i32          height;
    isize        size;
    ImageDataPtr data;
    ImageData() = default;
};

struct ImageHeader {
    // these two size is not for tex, just come from we
    // using Slot's size for tex
    i32 width;
    i32 height;
    i32 mapWidth;
    i32 mapHeight;

    bool mipmap_larger { false };
    bool mipmap_pow2 { false };

    ImageType     type { ImageType::UNKNOWN };
    TextureFormat format;
    i32           count;

    bool          isSprite;
    TextureSample sample;

    SpriteAnimation spriteAnim;
    // for specific property
    std::unordered_map<std::string, ImageExtra> extraHeader;
};

// slot is one singal image
struct Image : NoCopy, NoMove {
    struct Slot {
        i32 width;
        i32 height;

        std::vector<ImageData> mipmaps;

        operator bool() { return width * height * std::ssize(mipmaps) > 0; }
    };
    ImageHeader       header;
    std::vector<Slot> slots;
    std::string       key;
};

} // namespace wallpaper
