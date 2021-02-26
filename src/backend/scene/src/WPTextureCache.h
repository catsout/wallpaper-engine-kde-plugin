#pragma once
#include "GLWrapper.h"
#include <memory>

namespace wallpaper {
namespace gl {
class Texture {
public:
	Texture(GLWrapper* glWrapper, const std::string& name);
	Texture(GLWrapper* glWrapper, GLTexture* texture):glWrapper_(glWrapper),texture(texture) {};
	~Texture() {
		if (texture)
			glWrapper_->DeleteTexture(texture);
	}
	void Bind();
	GLTexture* texture;
	TextureFormat::Format Format() {return format_;};
	std::vector<int> GetResolution();
private:
	std::string name_;
	TextureFormat::Format format_;
	int resolution_[4] = {0};
	GLWrapper* glWrapper_;
};

class WPTextureCache {
public:
	WPTextureCache(GLWrapper* glWrapper):glWrapper_(glWrapper) {};
	Texture* LoadTexture(const std::string& name);
	Texture* GetTexture(const std::string& name);
	void Clear();
private:
	GLWrapper* glWrapper_;
	typedef std::unordered_map<std::string, std::unique_ptr<Texture>> TextureCache;
	TextureCache texCache_;
};

}
}
