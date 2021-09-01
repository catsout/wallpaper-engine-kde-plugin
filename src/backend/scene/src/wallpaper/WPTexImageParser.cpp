#include "WPTexImageParser.h"

#include <lz4.h>

#include "SpriteAnimation.h"
#include "Util.h"
#include "pkg.h"
#include "wallpaper.h"
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

int32_t ReadTexVesion(std::ifstream& file) {
    std::string str_v;
    str_v.resize(9);
    file.read(&str_v[0], 9);
    if (str_v.find("TEX") == std::string::npos)
        return 0;
    //	std::cout << str_v << std::endl;
    return std::stoi(str_v.c_str() + 4);
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
        LOG_ERROR("ERROR::ToTexFormate Unkown image type: " + std::to_string(type));
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

void LoadHeader(std::ifstream& file, ImageHeader& header) {
    int32_t unkown;
    header.extraHeader["texv"].val = ReadTexVesion(file);
    header.extraHeader["texi"].val = ReadTexVesion(file);
    header.format = ToTexFormate(readInt32(file));
    auto flags = LoadFlags(readInt32(file));
    {
        header.isSprite = flags.sprite;
        header.sample.wrapS = header.sample.wrapT = flags.clampUVs ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
        header.sample.minFilter = header.sample.magFilter = flags.noInterpolation ? TextureFilter::NEAREST : TextureFilter::LINEAR;
        header.extraHeader["compo1"].val = flags.compo1;
        header.extraHeader["compo2"].val = flags.compo2;
        header.extraHeader["compo3"].val = flags.compo3;
    }
    
    header.width = readInt32(file);
    header.height = readInt32(file);
    // in sprite this mean one pic
    header.mapWidth = readInt32(file);
    header.mapHeight = readInt32(file);
    unkown = readInt32(file);
    header.extraHeader["texb"].val = ReadTexVesion(file);
    header.count = readInt32(file);
    if (header.extraHeader["texb"].val == 3)
        header.type = static_cast<ImageType>(readInt32(file));
}

std::shared_ptr<Image> WPTexImageParser::Parse(const std::string& name) {
    std::string path = "materials/" + name + ".tex";
    std::shared_ptr<Image> img_ptr = std::make_shared<Image>();
    auto& img = *img_ptr;
    std::ifstream file = fs::GetFstream(WallpaperGL::GetPkgfs(), path);
    if (!file.is_open())
        return nullptr;
    auto startpos = file.tellg();
    LoadHeader(file, img.header);

    // image
    int32_t image_count = img.header.count;

    img.slots.resize(image_count);
    for (int32_t i_image = 0; i_image < image_count; i_image++) {
        auto& mipmaps = img.slots.at(i_image);

        int mipmap_count = readInt32(file);
        mipmaps.resize(mipmap_count);
        // load image
        for (int32_t i_mipmap = 0; i_mipmap < mipmap_count; i_mipmap++) {
            auto& mipmap = mipmaps.at(i_mipmap);
            mipmap.width = readInt32(file);
            mipmap.height = readInt32(file);
            if (i_mipmap == 0) {
                if(mipmap.width != img.header.width || mipmap.height != img.header.height) {
                    img.header.width = mipmap.width;
                    img.header.height = mipmap.height;
                }
            }

            bool LZ4_compressed = false;
            int32_t decompressed_size = 0;
            // check compress
            if (img.header.extraHeader["texb"].val > 1) {
                LZ4_compressed = readInt32(file) == 1;
                decompressed_size = readInt32(file);
            }
            int32_t src_size = readInt32(file);
            char* result;
            result = new char[src_size];
            file.read(result, src_size);

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
    std::string path = "materials/" + name + ".tex";
    std::ifstream file = fs::GetFstream(WallpaperGL::GetPkgfs(), path);
    if (!file.is_open()) 
        return header;

    LoadHeader(file, header);
    int32_t image_count = header.count;
    // load sprite info
    if (header.isSprite) {
        // bypass image data, store width and height
        std::vector<std::vector<float>> imageDatas(image_count);
        for (int32_t i_image = 0; i_image < image_count; i_image++) {
            int mipmap_count = readInt32(file);
            for (int32_t i_mipmap = 0; i_mipmap < mipmap_count; i_mipmap++) {
                int32_t width = readInt32(file);
                int32_t height = readInt32(file);
                if (i_mipmap == 0) {
                    imageDatas.at(i_image) = {(float)width, (float)height};
                    if(width != header.width || height != header.height) {
                        header.width = width;
                        header.height = height;
                    }
                }
                if (header.extraHeader["texb"].val > 1) {
                    int32_t LZ4_compressed = readInt32(file);
                    int32_t decompressed_size = readInt32(file);
                }
                std::streampos src_size = readInt32(file);
                file.seekg(file.tellg() + src_size);
            }
        }
        // sprite pos
        int32_t texs = ReadTexVesion(file);
        int32_t framecount = readInt32(file);
        if (texs > 3) {
            LOG_ERROR("Unkown texs version");
        }
        if (texs == 3) {
            int32_t width = readInt32(file);
            int32_t height = readInt32(file);
        }
        auto& spriteAnim = header.spriteAnim;
        for (int32_t i = 0; i < framecount; i++) {
            SpriteFrame sf;
            sf.imageId = readInt32(file);
            float spriteWidth = imageDatas.at(sf.imageId)[0];
            float spriteHeight = imageDatas.at(sf.imageId)[1];

            sf.frametime = ReadFloat(file);
            float width, height;
            if (texs == 1) {
                sf.x = readInt32(file) / spriteWidth;
                sf.y = readInt32(file) / spriteHeight;
                width = readInt32(file);
                sf.unk0 = readInt32(file);
                sf.unk1 = readInt32(file);
                height = readInt32(file);
            } else {
                sf.x = ReadFloat(file) / spriteWidth;
                sf.y = ReadFloat(file) / spriteHeight;
                width = ReadFloat(file);
                sf.unk0 = ReadFloat(file);
                sf.unk1 = ReadFloat(file);
                height = ReadFloat(file);
            }
            sf.width = width / spriteWidth;
            sf.height = height / spriteHeight;
            sf.rate = height / width;
            header.spriteAnim.AppendFrame(sf);
        }
    } else {
            int mipmap_count = readInt32(file);
            int32_t width = readInt32(file);
            int32_t height = readInt32(file);
            if(width != header.width || height != header.height) {
                header.width = width;
                header.height = height;
            }
 
    }
    return header;
}
