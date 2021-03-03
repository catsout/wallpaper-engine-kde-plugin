#include "WPTextureCache.h"
#include "teximage.h"
#include "pkg.h"
#include "wallpaper.h"

using namespace wallpaper::gl;


std::vector<float> SpriteFrame::GetTranslation(const SpriteFrame& sf) {
	return {sf.x, sf.y};
}
	
std::vector<float> SpriteFrame::GetRotation(const SpriteFrame& sf) {
	return {sf.width, 0.0f, 0.0f, sf.height};
}

Texture::Texture(GLWrapper* glWrapper,const std::string& name):m_glWrapper(glWrapper),m_name(name) {
	TexImage tex = TexImage::LoadFromFile("materials/" +name+".tex");
	LOG_INFO("load " + tex.Type() + std::string(tex.IsSprite()?"(sprite)":"") + " tex: " + name);

	tex.GetResolution(m_resolution);
	m_format = tex.Main().Format();
	m_isSprite = tex.IsSprite();
	if(m_isSprite) {
		m_spriteFrames = tex.SpriteFrames();
	}

	int img_count = tex.ImageNum();
	m_textures.resize(img_count);
	for(int i_img=0;i_img < img_count;i_img++) {
		auto& texture = m_textures[i_img];
		int mip_count = tex.MipmapNum(i_img);

		texture = m_glWrapper->CreateTexture(GL_TEXTURE_2D, tex.Main().Width(), tex.Main().Height(), mip_count);
		// mipmaps
		for(int i_mip=0;i_mip < mip_count;i_mip++){
			Image& img = tex.Mipmap(i_img, i_mip);
			m_glWrapper->TextureImage(texture, i_mip, img.Width(), img.Height(), m_format ,(uint8_t*)img.RawData(), true, img.Size());
		}
	}
	if(img_count > 0)
		texture = m_textures.at(0);
}

Texture::~Texture() {
	for(auto& t:m_textures) {
		if(t)
			m_glWrapper->DeleteTexture(t);
	}
}


const SpriteFrame* Texture::NextSpriteFrame(int time) {
	if(m_spriteFrames.size() == 0)
		return nullptr;
	if((m_frametime -= time) <= 0) {
		auto* sf = NextSpriteFrame();	
		m_frametime = (int)(sf->frametime*1000);
		return sf;
	}
	else return nullptr;
}

const SpriteFrame* Texture::NextSpriteFrame() {
	if(m_spriteFrames.size() == 0) {
//		LOG_ERROR("Use sprite but no sprite info data");
		return nullptr;
	}
	m_spriteIter++;
	if(m_spriteIter == m_spriteFrames.size()) {
		m_spriteIter = 0;
	}
	return &m_spriteFrames.at(m_spriteIter);
}

void Texture::SwitchTex(int index) {
	if(index >= Count()) {
		LOG_ERROR("Out of range for switching texture, " + std::to_string(index));
	}
	else {
		texture = m_textures[index];
	}
}

void Texture::Bind() {
	m_glWrapper->BindTexture(texture);
}


std::vector<int> Texture::GetResolution() {
	return std::vector<int>(m_resolution, m_resolution+4);
}

Texture* WPTextureCache::LoadTexture(const std::string& name) {
	if (m_texCache.count(name))
		return m_texCache[name].get();
	m_texCache[name] = std::make_unique<Texture>(m_glWrapper, name);
	return m_texCache[name].get();
}

Texture* WPTextureCache::GetTexture(const std::string& name) {
	if (m_texCache.count(name))
		return m_texCache[name].get();
	return nullptr;
}

void WPTextureCache::Clear() {
	m_texCache.clear();
}
