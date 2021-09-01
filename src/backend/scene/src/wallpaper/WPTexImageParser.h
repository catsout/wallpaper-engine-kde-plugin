#pragma once
#include "Interface/IImageParser.h"

namespace wallpaper
{

class WPTexImageParser : public IImageParser {
public:
	std::shared_ptr<Image> Parse(const std::string&) override;
	ImageHeader ParseHeader(const std::string&) override;
};
}
