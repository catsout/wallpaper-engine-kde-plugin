#pragma once

#include <filesystem>
#include "Fs.h"
#include "CBinaryStream.h"
#include "../Utils/Logging.h"

namespace wallpaper
{
namespace fs
{

class PhysicalFs : public Fs {
public:
	PhysicalFs(std::string_view physicalPath):m_path(physicalPath) {}
	virtual ~PhysicalFs() = default;
	virtual bool Contains(std::string_view path) const {
		auto fullpath = m_path / path.substr(1);
		return std::filesystem::exists(fullpath);
	}
	virtual std::shared_ptr<IBinaryStream> Open(std::string_view path) {
		if(Contains(path)) {
			return CreateCBinaryStream(FullPath(path));
		}
		return nullptr;
	}
private:
	std::string FullPath(std::string_view path) const {
		auto fullpath = m_path / path.substr(1);
		return fullpath.string();
	}
	std::filesystem::path m_path;
};

inline std::unique_ptr<PhysicalFs> CreatePhysicalFs(std::string_view path) {
	if(!std::filesystem::exists(path)) {
		LOG_ERROR("\"%s\" not exists", path.data());
		return nullptr;
	}
	return std::make_unique<PhysicalFs>(path);
}
}
}