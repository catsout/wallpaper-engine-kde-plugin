#pragma once
#include "image.h"
#include <vector>
#include <fstream>
#include <string>
namespace wallpaper
{
class TexImage
{
public:
	// init 1 mipmap, as TexImage always has one image
	TexImage():m_mipmap(1) {};
	static TexImage LoadFromFile(const std::string&);
	TexImage(TexImage&& other);

	Image& Main() {return m_mipmap.at(0);}
	const Image& Main() const {return m_mipmap.at(0);}

	Image& Mipmap(int index) {return m_mipmap.at(index);};
	const Image& Mipmap(int index) const {return m_mipmap.at(index);};

	int MipmapNum() {return m_mipmap.size();}
	bool Loaded() {return true;};
	void GetResolution(int*);
	std::string Type() const;
private:
	int m_texv;
	int m_texi;
	int m_texb;
	int m_width;
	int m_height;
	// sometimes textrue is bigger than picture, with black boader
	int m_pic_width;
	int m_pic_height;
	std::vector<Image> m_mipmap;
};
}
