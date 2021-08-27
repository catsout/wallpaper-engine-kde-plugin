#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include "Type.h"
#include "SpriteAnimation.h"
#include "Scene/SceneTexture.h"

namespace wallpaper
{

union ImageExtra {
	int32_t val;
	char str[125];
};

typedef std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> ImageDataPtr;

struct ImageData {
	uint32_t width;
	uint32_t height;
	uint32_t size;
	ImageDataPtr data;
	ImageData() = default;
	ImageData(const ImageData&) = delete;
	ImageData(ImageData&&) = default;
};

typedef std::vector<ImageData> MipmapDatas;

struct Image {
	uint32_t width;
	uint32_t height;
	uint32_t mapWidth;
	uint32_t mapHeight;
	ImageType type {ImageType::UNKNOWN};
	TextureFormat format;
	uint32_t count;
	bool isSprite;
	TextureSample sample;
	
	SpriteAnimation spriteAnim;

	// for specific property
	std::unordered_map<std::string, ImageExtra> extraHeader;
	std::vector<MipmapDatas> imageDatas;
};
}
