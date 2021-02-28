#include "WPTextureCache.h"
#include "teximage.h"
#include "pkg.h"
#include "wallpaper.h"

using namespace wallpaper::gl;

Texture::Texture(GLWrapper* glWrapper,const std::string& name):glWrapper_(glWrapper),name_(name) {
	TexImage tex = TexImage::LoadFromFile("materials/" +name+".tex");
	tex.GetResolution(resolution_);
	format_ = tex.Main().Format();
	texture = glWrapper_->CreateTexture(GL_TEXTURE_2D, tex.Main().Width(), tex.Main().Height(), tex.MipmapNum());
	for(int i=0;i<tex.MipmapNum();i++){
		Image& img = tex.Mipmap(i);
		glWrapper_->TextureImage(texture, i, img.Width(), img.Height(), format_ ,(uint8_t*)img.RawData(), false, img.Size());
	}
}

void Texture::Bind() {
	glWrapper_->BindTexture(texture);
}


std::vector<int> Texture::GetResolution() {
	return std::vector<int>(resolution_, resolution_+4);
}

Texture* WPTextureCache::LoadTexture(const std::string& name) {
	if (texCache_.count(name))
		return texCache_[name].get();
	texCache_[name] = std::make_unique<Texture>(glWrapper_, name);
	return texCache_[name].get();
}

Texture* WPTextureCache::GetTexture(const std::string& name) {
	if (texCache_.count(name))
		return texCache_[name].get();
	return nullptr;
}

void WPTextureCache::Clear() {
	texCache_.clear();
}
