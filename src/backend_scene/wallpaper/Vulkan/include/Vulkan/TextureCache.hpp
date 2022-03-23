#pragma once

#include "Parameters.hpp"
#include "Type.h"
#include "Utils/NoCopyMove.hpp"
#include "Utils/MapSet.hpp"

namespace wallpaper
{

class Device;
class Image;

namespace vulkan
{

vk::Format ToVkType(TextureFormat);
vk::SamplerAddressMode ToVkType(TextureWrap); 
vk::Filter ToVkType(TextureFilter);

enum class TexUsage {
	COLOR,
	DEPTH
};

using TexHash=std::size_t;

struct TextureKey {
	uint16_t width;
	uint16_t height;
	TexUsage usage;
	TextureFormat format;
	TextureSample sample;
    uint mipmap_level {1};

	static TexHash HashValue(const TextureKey&);
};

class TextureCache : NoCopy,NoMove {
public:
    TextureCache(const Device&);
    ~TextureCache();

    void Destroy();
    void Clear();

    vk::ResultValue<ExImageParameters> CreateExTex(uint32_t witdh, uint32_t height, vk::Format, vk::ImageTiling);
    vk::ResultValue<ImageSlots> CreateTex(Image&);

    vk::ResultValue<ImageParameters> Query(std::string_view key, TextureKey content_hash, bool persist=false);
    void MarkShareReady(std::string_view key);

    void RecGenerateMipmaps(vk::CommandBuffer& cmd, const ImageParameters& image) const;
private:
    vk::ResultValue<ImageParameters> CreateTex(TextureKey);
    void allocateCmd();
    vk::CommandBuffer m_tex_cmd;

    const Device& m_device;
    Map<std::string, ImageSlots> m_tex_map;

    struct QueryTex {
        int index {0};
        bool share_ready {false};
        bool persist {false};
        TexHash content_hash;
        ImageParameters image;
        Set<std::string> query_keys;
    };
    std::vector<std::unique_ptr<QueryTex>> m_query_texs;
    Map<std::string, QueryTex*> m_query_map;
};

}
}