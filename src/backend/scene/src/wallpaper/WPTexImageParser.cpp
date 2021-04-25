#include "WPTexImageParser.h"

#include "common.h"
#include "SpriteAnimation.h"

#include "wallpaper.h"
#include "pkg.h"

#include <lz4.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cstring>
#include <iostream>



using namespace wallpaper;


char* Lz4Decompress(const char* src, int size, int decompressed_size) {
	char* dst = new char[decompressed_size];
	int load_size = LZ4_decompress_safe(src, dst, size, decompressed_size);
	if (load_size < decompressed_size) {
		LOG_ERROR("lz4 decompress failed");
		delete [] dst;
		return nullptr;
	}
	return dst;
}

int32_t ReadTexVesion(std::ifstream& file) {
	std::string str_v;
    str_v.resize(9);
    file.read(&str_v[0],9);
	if(str_v.find("TEX") == std::string::npos)
		return 0;
//	std::cout << str_v << std::endl;	
	return std::stoi(str_v.c_str()+4);
}

TextureFormat ToTexFormate(int type) {
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
			return TextureFormat::RGBA8;
		case 4:
			return TextureFormat::BC3;
		case 6:
			return TextureFormat::BC2;
		case 7:
			return TextureFormat::BC1;
		case 8:
		case 9:
			LOG_ERROR("ERROR::ToTexFormate Unsupported image type: " + std::to_string(type));
		default:
			LOG_ERROR("ERROR::ToTexFormate Unkown image type: " + std::to_string(type));
			return TextureFormat::RGBA8;
	}
}
struct WPTexFlag {
	// true for no bilinear
	bool noInterpolation {false};
	// true for no repeat
	bool clampUVs {false};
	bool sprite {false};
};

WPTexFlag LoadFlags(int32_t value) {
	WPTexFlag flags;
	std::vector<bool*> values({&flags.noInterpolation,
							   &flags.clampUVs,
							   &flags.sprite});
	for(auto& el:values) {
		(*el) = value % 2;
		value /= 2;
	}
	return flags;
}

void LoadHeader(std::ifstream& file, Image& image) {
	int32_t unkown;
	image.extraHeader["texv"].val = ReadTexVesion(file);
	image.extraHeader["texi"].val = ReadTexVesion(file);
	image.format = ToTexFormate(readInt32(file));
	auto flags = LoadFlags(readInt32(file));
    image.isSprite = flags.sprite;
	image.sample.wrapS = image.sample.wrapT = flags.clampUVs?TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
	image.sample.minFilter = image.sample.magFilter = flags.noInterpolation?TextureFilter::NEAREST : TextureFilter::LINEAR;
    image.width = readInt32(file);
    image.height = readInt32(file);
    // in sprite this mean one pic 
    image.mapWidth = readInt32(file);
    image.mapHeight = readInt32(file);
    unkown = readInt32(file);
    image.extraHeader["texb"].val = ReadTexVesion(file);
	image.count = readInt32(file);
    if(image.extraHeader["texb"].val == 3)
        image.type = static_cast<ImageType>(readInt32(file));
}

std::shared_ptr<Image> WPTexImageParser::Parse(const std::string& name) {
	std::string path = "materials/" +name+".tex";
	std::shared_ptr<Image> img_ptr = std::make_shared<Image>();
	auto& img = *img_ptr;
	std::ifstream file = fs::GetFstream(WallpaperGL::GetPkgfs(), path);
	if(!file.is_open()) return nullptr;
	auto startpos = file.tellg();
	LoadHeader(file, img);

	// image 
	int32_t image_count = img.count;

	img.imageDatas.resize(image_count);
	for(int32_t i_image = 0;i_image < image_count;i_image++) {
		auto& mipmaps = img.imageDatas.at(i_image);

		int mipmap_count = readInt32(file);
		mipmaps.resize(mipmap_count);
		// load image
		for(int32_t i_mipmap = 0; i_mipmap < mipmap_count; i_mipmap++) {
			auto& mipmap = mipmaps.at(i_mipmap);
			mipmap.width = readInt32(file);
			mipmap.height = readInt32(file);

			bool LZ4_compressed = false;
			int32_t decompressed_size = 0;
			// check compress	
			if(img.extraHeader["texb"].val > 1)
			{
				LZ4_compressed = readInt32(file) == 1;
				decompressed_size = readInt32(file);
			}   
			int32_t src_size = readInt32(file);
			char* result;
			result = new char[src_size];file.read(result,src_size);
			
			// is LZ4 compress
			if(LZ4_compressed) {
				char* decompressed_char = Lz4Decompress(result, src_size, decompressed_size);
				src_size = decompressed_size;
				if(decompressed_char != nullptr) {	
					delete [] result;
					result = decompressed_char;
				}
				else { 
					LOG_ERROR("lz4 decompress failed");
					delete [] result;
					return nullptr;
				}
			}
			// is image container
			if(img.extraHeader["texb"].val == 3 && (int32_t)img.type != -1) {
				int32_t w,h,n;
				auto* data = stbi_load_from_memory((const unsigned char*)result, src_size, &w, &h, &n, 4);
				mipmap.data = ImageDataPtr((uint8_t*)data,
					[](uint8_t* data) { stbi_image_free((unsigned char*)data); });
				src_size = w*h*4*sizeof(uint8_t);
			}
			else {
				mipmap.data = ImageDataPtr(new uint8_t[src_size], 
					[](uint8_t* data) { delete [] data; });
				memcpy(mipmap.data.get(), result, src_size);
			}
			mipmap.size = src_size;
			delete [] result;
		}
	}
	return img_ptr;
}

std::shared_ptr<Image> WPTexImageParser::ParseHeader(const std::string& name) {
	std::string path = "materials/" +name+".tex";
	auto img_ptr = std::make_shared<Image>();
	auto& img = *img_ptr.get();
	std::ifstream file = fs::GetFstream(WallpaperGL::GetPkgfs(), path);
	if(!file.is_open()) return img_ptr;
	LoadHeader(file, img);
	int32_t image_count = img.count;
	// load sprite info
	if(img.isSprite) {
		// bypass image data, store width and height
		std::vector<std::vector<float>> imageDatas(image_count);
		for(int32_t i_image = 0;i_image < image_count;i_image++) {
			int mipmap_count = readInt32(file);
			for(int32_t i_mipmap = 0; i_mipmap < mipmap_count; i_mipmap++) {
				float width = readInt32(file);
				float height = readInt32(file);
				if(i_mipmap == 0)
					imageDatas.at(i_image) = {width, height};
				if(img.extraHeader["texb"].val > 1) {
					int32_t LZ4_compressed = readInt32(file);
					int32_t decompressed_size = readInt32(file);
				}   
				std::streampos src_size = readInt32(file);
				file.seekg(file.tellg() + src_size);
			}
		}
		// sprite pos
		int32_t texs = ReadTexVesion(file);
		int32_t framecount = readInt32(file);
		if(texs > 3) {
			LOG_ERROR("Unkown texs version");
		}
		if(texs == 3) {
			int32_t width = readInt32(file);
            int32_t height = readInt32(file); 
		}
		auto& spriteAnim = img.spriteAnim;
		for(int32_t i=0;i < framecount;i++) {
			SpriteFrame	sf;
			sf.imageId = readInt32(file);
			float spriteWidth = imageDatas.at(sf.imageId)[0];
			float spriteHeight = imageDatas.at(sf.imageId)[1];

			sf.frametime = ReadFloat(file); 
			if(texs == 1) {
				sf.x = readInt32(file) / spriteWidth; 
				sf.y = readInt32(file) / spriteHeight; 
				sf.width = readInt32(file) / spriteWidth; 
				sf.unk0 = readInt32(file); 
				sf.unk1 = readInt32(file); 
				sf.height = readInt32(file) / spriteHeight; 

			} else {
				sf.x = ReadFloat(file) / spriteWidth; 
				sf.y = ReadFloat(file) / spriteHeight; 
				sf.width = ReadFloat(file) / spriteWidth; 
				sf.unk0 = ReadFloat(file); 
				sf.unk1 = ReadFloat(file); 
				sf.height = ReadFloat(file) / spriteHeight; 
			}
			img.spriteAnim.AppendFrame(sf);
		}
	}
	return img_ptr;
}
