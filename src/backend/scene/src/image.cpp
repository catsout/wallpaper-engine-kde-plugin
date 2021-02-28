#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

using namespace wallpaper;


std::string ImageType::to_string(ImageType::Type type) {
#define IMG(str) case str: return #str;
	switch(type) {
        IMG(UNKNOWN)
        IMG(BMP)
        IMG(ICO)
		IMG(JPEG)
        IMG(JNG)
        IMG(PNG)
		default:
			LOG_ERROR("Not valied image type: " + std::to_string((int)type));
			return "";
	}
}

Image::Image(const char* file_data, int size, ImageType::Type type):m_type(type) {
	int num;
	//stbi_set_flip_vertically_on_load(true);
	char* data = (char*)stbi_load_from_memory((const unsigned char*)file_data, size, &m_width, &m_height, &num, 4);
	m_data = std::shared_ptr<char>(data, [](char* data){stbi_image_free(data);});
	if(m_data == NULL)
		LOG_ERROR("ERROR::Image stb load image failed");
	m_format = gl::TextureFormat::RGBA8;
	m_size = size;
}
