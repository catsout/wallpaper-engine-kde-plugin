#pragma once

#include <filesystem>
#include "Fs.h"
#include "CBinaryStream.h"
#include "Utils/Logging.h"

namespace wallpaper
{
namespace fs
{

class PhysicalFs : public Fs {
public:
    PhysicalFs(std::string_view physicalPath): m_path(physicalPath) {}
    virtual ~PhysicalFs() = default;
    bool Contains(std::string_view path) const override {
        auto fullpath = m_path / path.substr(1);
        return std::filesystem::exists(fullpath);
    }
    std::shared_ptr<IBinaryStream> Open(std::string_view path) override {
        return CreateCBinaryStream(FullPath(path));
    }
    std::shared_ptr<IBinaryStreamW> OpenW(std::string_view path) override {
        std::filesystem::path full_path { FullPath(path) };
        std::filesystem::create_directories(full_path.parent_path());
        return CreateCBinaryStreamW(full_path.native());
    }

private:
    std::string FullPath(std::string_view path) const {
        auto fullpath = m_path / path.substr(1);
        return fullpath.string();
    }
    std::filesystem::path m_path;
};

inline std::unique_ptr<PhysicalFs> CreatePhysicalFs(std::string_view path, bool create = false) {
    if (! std::filesystem::exists(path)) {
        if (create) {
            if (! std::filesystem::create_directories(path)) {
                LOG_ERROR("mkdir \"%s\" failed", path.data());
                return nullptr;
            }
        } else {
            LOG_ERROR("\"%s\" not exists", path.data());
            return nullptr;
        }
    }
    return std::make_unique<PhysicalFs>(path);
}
} // namespace fs
} // namespace wallpaper
