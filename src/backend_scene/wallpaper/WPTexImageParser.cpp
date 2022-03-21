#include "WPTexImageParser.h"


#include "WPCommon.hpp"
#include <lz4.h>

#include "SpriteAnimation.h"
#include "Algorism.h"
#include "Fs/VFS.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstring>
#include <iostream>

using namespace wallpaper;

char* Lz4Decompress(const char* src, int size, int decompressed_size) {
    char* dst = new char[decompressed_size];
    int load_size = LZ4_decompress_safe(src, dst, size, decompressed_size);
    if (load_size < decompressed_size) {
        LOG_ERROR("lz4 decompress failed");
        delete[] dst;
        return nullptr;
    }
    return dst;
}


TextureFormat ToTexFormate(int type) {
    /*  
		type
        RGBA8888 = 0,
        DXT5 = 4,
        DXT3 = 6,
        DXT1 = 7,
        RG88 = 8,
        R8 = 9,
*/
    switch (type) {
    case 0:
        return TextureFormat::RGBA8;
    case 4:
        return TextureFormat::BC3;
    case 6:
        return TextureFormat::BC2;
    case 7:
        return TextureFormat::BC1;
    case 8:
        return TextureFormat::RG8;
    case 9:
        return TextureFormat::R8;
    default:
        LOG_ERROR("ERROR::ToTexFormate Unkown image type: %d", type);
        return TextureFormat::RGBA8;
    }
}
struct WPTexFlag {
	static constexpr int Num {6};
	static constexpr const std::array<uint32_t, Num> Masks {
		1u,
		1u<<1,
		1u<<2,
		1u<<20,
		1u<<21,
		1u<<22
	};
    // true for no bilinear
    bool noInterpolation {false};
    // true for no repeat
    bool clampUVs {false};
    bool sprite {false};

	bool compo1 {false};
	bool compo2 {false};
	bool compo3 {false};
};

WPTexFlag LoadFlags(uint32_t value) {
    WPTexFlag flags;
    std::array<bool*, WPTexFlag::Num> values({
		&flags.noInterpolation,
        &flags.clampUVs,
        &flags.sprite,
		&flags.compo1,
		&flags.compo2,
		&flags.compo3
	});
	for(int i=0;i<WPTexFlag::Num;i++) {
		*values[i] = (value&WPTexFlag::Masks[i]) > 0u;
	}
    return flags;
}

void LoadHeader(fs::IBinaryStream& file, ImageHeader& header) {
    int32_t unkown;
    header.extraHeader["texv"].val = ReadTexVesion(file);
    header.extraHeader["texi"].val = ReadTexVesion(file);
    header.format = ToTexFormate(file.ReadInt32());
    auto flags = LoadFlags(file.ReadInt32());
    {
        header.isSprite = flags.sprite;
        header.sample.wrapS = header.sample.wrapT = flags.clampUVs ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
        header.sample.minFilter = header.sample.magFilter = flags.noInterpolation ? TextureFilter::NEAREST : TextureFilter::LINEAR;
        header.extraHeader["compo1"].val = flags.compo1;
        header.extraHeader["compo2"].val = flags.compo2;
        header.extraHeader["compo3"].val = flags.compo3;
    }

    /*
        picture:
        width, height --> pow of 2 (tex size)
        mapw, maph    --> pic size
        mips
        mipw,miph     --> pow of 2

        sprites:
        width, height --> piece of sprite sheet
        mapw, maph    --> same
        1 mip
        mipw,mimp     --> tex size
    */
    
    header.width = file.ReadInt32();
    header.height = file.ReadInt32();
    // in sprite this mean one pic
    header.mapWidth = file.ReadInt32();
    header.mapHeight = file.ReadInt32();
    unkown = file.ReadInt32();
    header.extraHeader["texb"].val = ReadTexVesion(file);
    header.count = file.ReadInt32();
    if (header.extraHeader["texb"].val == 3)
        header.type = static_cast<ImageType>(file.ReadInt32());
}

std::shared_ptr<Image> WPTexImageParser::Parse(const std::string& name) {
    std::string path = "/assets/materials/" + name + ".tex";
    std::shared_ptr<Image> img_ptr = std::make_shared<Image>();
    auto& img = *img_ptr;
    img.key = name;
    //std::ifstream file = fs::GetFileFstream(vfs, path);
    auto pfile = m_vfs->Open(path);
    if (!pfile)
        return nullptr;
    auto& file = *pfile;
    auto startpos = file.Tell();
    LoadHeader(file, img.header);

    // image
    int32_t image_count = img.header.count;

    img.slots.resize(image_count);
    for (int32_t i_image = 0; i_image < image_count; i_image++) {
        auto& img_slot = img.slots[i_image];
        auto& mipmaps = img_slot.mipmaps;

        int mipmap_count = file.ReadInt32();
        mipmaps.resize(mipmap_count);
        // load image
        for (int32_t i_mipmap = 0; i_mipmap < mipmap_count; i_mipmap++) {
            auto& mipmap = mipmaps.at(i_mipmap);
            mipmap.width = file.ReadInt32();
            mipmap.height = file.ReadInt32();
            if (i_mipmap == 0) {
                img_slot.width = mipmap.width;
                img_slot.height = mipmap.height;
                /*
                LOG_INFO("1: %dx%d, 2: %dx%d, 3: %dx%d, level: %d", img.header.width, img.header.height,
                    img.header.mapWidth, img.header.mapHeight,
                    mipmap.width,mipmap.height,
                    mipmap_count);
                */
            }

            bool LZ4_compressed = false;
            int32_t decompressed_size = 0;
            // check compress
            if (img.header.extraHeader["texb"].val > 1) {
                LZ4_compressed = file.ReadInt32() == 1;
                decompressed_size = file.ReadInt32();
            }
            int32_t src_size = file.ReadInt32();
            if(src_size <= 0 || mipmap.width <=0 || mipmap.height <=0 || decompressed_size<0)
                return nullptr;
            char* result;
            result = new char[src_size];
            file.Read(result, src_size);

            // is LZ4 compress
            if (LZ4_compressed) {
                char* decompressed_char = Lz4Decompress(result, src_size, decompressed_size);
                src_size = decompressed_size;
                if (decompressed_char != nullptr) {
                    delete[] result;
                    result = decompressed_char;
                } else {
                    LOG_ERROR("lz4 decompress failed");
                    delete[] result;
                    return nullptr;
                }
            }
            // is image container
            if (img.header.extraHeader["texb"].val == 3 && (int32_t)img.header.type != -1) {
                int32_t w, h, n;
                auto* data = stbi_load_from_memory((const unsigned char*)result, src_size, &w, &h, &n, 4);
                mipmap.data = ImageDataPtr((uint8_t*)data,
                                           [](uint8_t* data) { stbi_image_free((unsigned char*)data); });
                src_size = w * h * 4;
            } else {
                mipmap.data = ImageDataPtr(new uint8_t[src_size],
                                           [](uint8_t* data) { delete[] data; });
                memcpy(mipmap.data.get(), result, src_size * sizeof(uint8_t));
            }
            mipmap.size = src_size * sizeof(uint8_t);
            delete[] result;
        }
    }
    return img_ptr;
}

ImageHeader WPTexImageParser::ParseHeader(const std::string& name) {
	ImageHeader header;
    std::string path = "/assets/materials/" + name + ".tex";
    auto pfile = m_vfs->Open(path);
    if (!pfile)
        return header;
    auto& file = *pfile;

    LoadHeader(file, header);
    int32_t image_count = header.count;
    // load sprite info
    if (header.isSprite) {
        // bypass image data, store width and height
        std::vector<std::vector<float>> imageDatas(image_count);
        for (int32_t i_image = 0; i_image < image_count; i_image++) {
            int mipmap_count = file.ReadInt32();
            for (int32_t i_mipmap = 0; i_mipmap < mipmap_count; i_mipmap++) {
                int32_t width = file.ReadInt32();
                int32_t height = file.ReadInt32();
                if (i_mipmap == 0) {
                    imageDatas.at(i_image) = {(float)width, (float)height};
                    header.mipmap_pow2 = algorism::IsPowOfTwo(width*height);
                }
                if (header.extraHeader["texb"].val > 1) {
                    int32_t LZ4_compressed = file.ReadInt32();
                    int32_t decompressed_size = file.ReadInt32();
                }
                long src_size = file.ReadInt32();
                file.SeekCur(src_size);
            }
        }
        // sprite pos
        int32_t texs = ReadTexVesion(file);
        int32_t framecount = file.ReadInt32();
        if (texs > 3) {
            LOG_ERROR("Unkown texs version");
        }
        if (texs == 3) {
            int32_t width = file.ReadInt32();
            int32_t height = file.ReadInt32();
        }
        auto& spriteAnim = header.spriteAnim;
        for (int32_t i = 0; i < framecount; i++) {
            SpriteFrame sf;
            sf.imageId = file.ReadInt32();
            float spriteWidth = imageDatas.at(sf.imageId)[0];
            float spriteHeight = imageDatas.at(sf.imageId)[1];

            sf.frametime = file.ReadFloat();
            if (texs == 1) {
                sf.x = file.ReadInt32() / spriteWidth;
                sf.y = file.ReadInt32() / spriteHeight;
                sf.xAxis[0] = file.ReadInt32();
                sf.xAxis[1] = file.ReadInt32();
                sf.yAxis[0] = file.ReadInt32();
                sf.yAxis[1] = file.ReadInt32();
            } else {
                sf.x = file.ReadFloat() / spriteWidth;
                sf.y = file.ReadFloat() / spriteHeight;
                sf.xAxis[0] = file.ReadFloat();
                sf.xAxis[1] = file.ReadFloat();
                sf.yAxis[0] = file.ReadFloat();
                sf.yAxis[1] = file.ReadFloat();
            }
            sf.width = std::sqrt(std::pow(sf.xAxis[0],2) + std::pow(sf.xAxis[1],2));
            sf.height = std::sqrt(std::pow(sf.yAxis[0],2) + std::pow(sf.yAxis[1],2));
            sf.xAxis[0] /= spriteWidth;
            sf.xAxis[1] /= spriteWidth;
            sf.yAxis[0] /= spriteHeight;
            sf.yAxis[1] /= spriteHeight;
            sf.rate = sf.height / sf.width;
            header.spriteAnim.AppendFrame(sf);
        }
    } else {
            int mipmap_count = file.ReadInt32();
            int32_t width = file.ReadInt32();
            int32_t height = file.ReadInt32();
            header.mipmap_pow2 = algorism::IsPowOfTwo(width*height);
    }
    return header;
}
