#pragma once
#include "IBinaryStream.h"

namespace wallpaper
{
namespace fs
{

class Fs {
public:
	virtual bool Contains(std::string_view path) const = 0;
	virtual std::shared_ptr<IBinaryStream> Open(std::string_view path) = 0;
public:
	Fs() = default;
	virtual ~Fs() = default;
	Fs(Fs&&) = default;
	Fs& operator=(Fs&&) = default;

	Fs(const Fs&) = delete;
	Fs& operator=(const Fs&) = delete;
};

}
}