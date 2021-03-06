#pragma once
#include <vector>
#include <fstream>
#include <string>

#include "image.h"
#include "WPTextureCache.h"

namespace wallpaper
{

struct TexFlag {
	bool noInterpolation;
	bool clampUVs;
	bool sprite;
	static TexFlag LoadFlags(int value);
};

struct TexHeader {
	int texv;
	int texi;
	gl::TextureFormat::Format type;
    TexFlag flags;
    int width;
    int height;
    int pic_width;
    int pic_height;
    int unkown;
    int texb;
	static TexHeader ReadTexHeader(std::ifstream&);
};

class TexImage
{
public:
	// init 1 mipmap, as TexImage always has one image
	TexImage():m_mipmap(1) {};
	static TexImage LoadFromFile(const std::string&);
	TexImage(TexImage&& other);

	Image& Main() {return m_mipmap.at(0).at(0);}
	const Image& Main() const {return m_mipmap.at(0).at(0);}

	Image& Mipmap(int imgIndex, int index) {return m_mipmap.at(imgIndex).at(index);};
	const Image& Mipmap(int imgIndex, int index) const {return m_mipmap.at(imgIndex).at(index);};

	Image& Mipmap(int index) {return Mipmap(0, index);};
	const Image& Mipmap(int index) const {return Mipmap(0, index);};

	int ImageNum() const {return m_mipmap.size();};
	int MipmapNum(int imgIndex) const {return m_mipmap.at(imgIndex).size();};
	int MipmapNum() const {return MipmapNum(0);};

	bool Loaded() {return true;};
	bool IsSprite() const  {return m_header.flags.sprite;};
	bool PointFilter() const {return m_header.flags.noInterpolation;};
	bool ClampEdge() const {return m_header.flags.clampUVs;};
	void GetResolution(int*);
	std::string Type() const;

	const std::vector<gl::SpriteFrame>& SpriteFrames() {return m_spriteFrames;};

private:
	TexHeader m_header;
	std::vector<std::vector<Image>> m_mipmap;
	std::vector<gl::SpriteFrame> m_spriteFrames;
};
}
