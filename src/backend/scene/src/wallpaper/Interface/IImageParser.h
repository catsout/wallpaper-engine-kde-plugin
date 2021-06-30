#pragma once
#include "Image.h"
#include <memory>
#include <string>

namespace wallpaper
{

class IImageParser {
public:
	IImageParser() = default;
	virtual ~IImageParser() = default;
	virtual std::shared_ptr<Image> Parse(const std::string&) = 0;
	virtual std::shared_ptr<Image> ParseHeader(const std::string&) = 0;
};
}
