#include "teximage.h"
#include "common.h"
#include "wallpaper.h"
#include "pkg.h"
#include <iostream>
#include <memory>
#include <lz4.h>

using namespace wallpaper;

TexImage::TexImage(TexImage && other):m_texv(other.m_texv),
	m_texi(other.m_texi),
	m_texb(other.m_texb),
	m_width(other.m_width),
	m_height(other.m_height),
	m_pic_width(other.m_pic_width),
	m_pic_height(other.m_pic_height),
	m_mipmap(std::move(other.m_mipmap)) 
	{}

gl::TextureFormat::Format ToTexFormate(int type)
{
/*  
		type
        RGBA8888 = 0,
        DXT5 = 4,
        DXT3 = 6,
        DXT1 = 7,
        RG88 = 8,
        R8 = 9,
*/
	switch(type)
	{
		case 0:
			return gl::TextureFormat::RGBA8;
		case 4:
			return gl::TextureFormat::BC3;
		case 6:
			return gl::TextureFormat::BC2;
		case 7:
			return gl::TextureFormat::BC1;
		case 8:
		case 9:
			LOG_ERROR("ERROR::ToTexFormate Unsupported image type: " + std::to_string(type));
		default:
			LOG_ERROR("ERROR::ToTexFormate Unkown image type: " + std::to_string(type));
			return gl::TextureFormat::RGBA8;
	}
}

char* Lz4Decompress(const char* src, int size, int decompressed_size)
{
	char* dst = new char[decompressed_size];
	int load_size = LZ4_decompress_safe(src, dst, size, decompressed_size);
	if (load_size < decompressed_size)
	{
		LOG_ERROR("lz4 decompress failed");
		delete [] dst;
		return nullptr;
	}
	return dst;
}

int ReadTexVesion(std::ifstream& file)
{
	std::string str_v;
    str_v.resize(9);
    file.read(&str_v[0],9);
	if(str_v.find("TEX") == std::string::npos)
		return 0;
//	std::cout << str_v << std::endl;	
	return std::stoi(str_v.c_str()+4);
}


void TexImage::GetResolution(int* size) {
	size[0] = Main().Width();
	size[1] = Main().Height();
	size[2] = m_pic_width;
	size[3] = m_pic_height;
}

TexImage TexImage::LoadFromFile(const std::string& path)
{
	TexImage tex;
	std::ifstream file = fs::GetFstream(WallpaperGL::GetPkgfs(), path);
	if(!file.is_open()) return tex;
	//tex vesion
	tex.m_texv = ReadTexVesion(file);
	tex.m_texi = ReadTexVesion(file);
	gl::TextureFormat::Format type = ToTexFormate(readInt32(file));

    //read header
    int flags,texwidth,texheight,width,height,unkown;
    flags = readInt32(file);
    tex.m_width = readInt32(file);
    tex.m_height = readInt32(file);
    tex.m_pic_width = readInt32(file);
    tex.m_pic_height = readInt32(file);
    unkown = readInt32(file);
    tex.m_texb = ReadTexVesion(file);
	// image 
	int image_count = 0,free_image_format = -1,mipmap_count = 0;
	image_count = readInt32(file);
    if(tex.m_texb == 3) {
        free_image_format = readInt32(file);
	}
    mipmap_count = readInt32(file);
	tex.m_mipmap.resize(mipmap_count);
	// load image
	for(int i_mipmap = 0; i_mipmap < mipmap_count; i_mipmap++)
	{
		int mipmap_width = readInt32(file);
		int mipmap_height = readInt32(file);

		bool LZ4_compressed = false;
		int decompressed_size = 0;
		// check compress	
		if(tex.m_texb > 1)
		{
			LZ4_compressed = readInt32(file) == 1;
			decompressed_size = readInt32(file);
		}   
		int src_size = readInt32(file);
		if(!decompressed_size)
			decompressed_size = src_size;
		char* result;
		result = new char[src_size];file.read(result,src_size);
		
		// is LZ4 compress
		if(LZ4_compressed)
		{
			char* decompressed_char = Lz4Decompress(result, src_size, decompressed_size);
			if(decompressed_char != nullptr) {	
				delete [] result;
				result = decompressed_char;
			}
			else LOG_ERROR("lz4 decompress failed");
		}
		auto imageType = static_cast<ImageType::Type>(free_image_format);
		// is image container
		if(tex.m_texb == 3 && free_image_format != -1)
		{
			tex.m_mipmap[i_mipmap] = (Image(result, decompressed_size, imageType));
			delete [] result;
		}
		else
			tex.m_mipmap[i_mipmap] = (Image(mipmap_width, mipmap_height, type, imageType, result, decompressed_size, [](char* data){delete [] data;}));	
	}
	return tex;
}

std::string TexImage::Type() const {
	if(Main().Type() == ImageType::UNKNOWN) {
		return gl::TextureFormat::to_string(Main().Format());	
	}
	else return ImageType::to_string(Main().Type());
}
