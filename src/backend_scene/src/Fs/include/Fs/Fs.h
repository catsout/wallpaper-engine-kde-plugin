#pragma once
#include <memory>

#include "IBinaryStream.h"
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{
namespace fs
{

class Fs : NoCopy,NoMove {
public:
	virtual bool Contains(std::string_view path) const = 0;
	virtual std::shared_ptr<IBinaryStream> Open(std::string_view path) = 0;
	virtual std::shared_ptr<IBinaryStreamW> OpenW(std::string_view path) = 0;
public:
	Fs() = default;
	virtual ~Fs() = default;
};

}
}
