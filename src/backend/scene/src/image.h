#pragma once
#include <memory>
#include <string>

#include <iostream>
#include "GLWrapper.h"

namespace wallpaper
{
class Image
{
public:
	Image() {};
	Image(const char* file_data, int size);

	template<class Deleter>
		Image(int width, int height, gl::TextureFormat::Format format, char* rawdata, size_t size, Deleter deleter):m_width(width),m_height(height),m_format(format),m_size(size),m_data(rawdata, deleter) {
	}


	int Width() const {return m_width;}
	int Height() const {return m_height;}
	gl::TextureFormat::Format Format() const {return m_format;}

	// only used for opengl api
	const char* RawData() const {return m_data.get();};
	size_t Size() const {return m_size;}
private:
	int m_width;
	int m_height;
	gl::TextureFormat::Format m_format;
	std::shared_ptr<char> m_data;
	size_t m_size;
};
}
