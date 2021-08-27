#pragma once

#include "TextureResource.h"
#include <vector>
#include <string>

namespace wallpaper {
namespace fg {

struct RenderTargetDesc {
	std::string name;
	TextureResource color;
};

struct RenderTargetResource : public Resource {
	void Initialize() {}; 
	bool Initialed() const { return true; };

	RenderTargetHandle handle;
	RenderTargetDesc desc;
};

}
}