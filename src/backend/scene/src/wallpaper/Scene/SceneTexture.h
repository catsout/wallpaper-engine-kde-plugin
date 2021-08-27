#pragma once
#include "SpriteAnimation.h"
#include <string>
#include <vector>
#include "Type.h"


namespace wallpaper
{

struct SceneTexture {
	std::string url;
	TextureSample sample;
	bool isSprite {false};
	SpriteAnimation spriteAnim;
};
}
