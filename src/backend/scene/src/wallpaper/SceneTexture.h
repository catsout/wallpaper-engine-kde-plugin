#pragma once
#include "SpriteAnimation.h"
#include <string>
#include <vector>


namespace wallpaper
{

enum class TextureWrap {
	CLAMP_TO_EDGE,
	REPEAT
};

enum class TextureFilter {
	LINEAR,
	NEAREST
};

struct SceneTextureSample {
	TextureWrap wrapS {TextureWrap::REPEAT};
	TextureWrap wrapT {TextureWrap::REPEAT};
	TextureFilter magFilter {TextureFilter::NEAREST};
	TextureFilter minFilter {TextureFilter::NEAREST};
};

struct SceneTexture {
	std::string url;
	SceneTextureSample sample;
	bool isSprite {false};
	SpriteAnimation spriteAnim;
};
}
