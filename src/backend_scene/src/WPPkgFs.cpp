#include "WPPkgFs.hpp"
#include "Utils/Logging.h"
#include "Fs/LimitedBinaryStream.h"
#include "Fs/CBinaryStream.h"
#include <vector>

using namespace wallpaper;
using namespace wallpaper::fs;

namespace
{
std::string ReadSizedString(IBinaryStream& f) {
    idx ilen = f.ReadInt32();
    assert(ilen >= 0);

    usize       len = (usize)ilen;
    std::string result;
    result.resize(len);
    f.Read(result.data(), len);
    return result;
}
} // namespace

std::unique_ptr<WPPkgFs> WPPkgFs::CreatePkgFs(std::string_view pkgpath) {
    auto ppkg = fs::CreateCBinaryStream(pkgpath);
    if (! ppkg) return nullptr;

    auto&       pkg = *ppkg;
    std::string ver = ReadSizedString(pkg);
    LOG_INFO("pkg version: %s", ver.data());

    std::vector<PkgFile> pkgfiles;
    i32                  entryCount = pkg.ReadInt32();
    for (i32 i = 0; i < entryCount; i++) {
        std::string path   = "/" + ReadSizedString(pkg);
        idx         offset = pkg.ReadInt32();
        idx         length = pkg.ReadInt32();
        pkgfiles.push_back({ path, offset, length });
    }
    auto pkgfs       = std::unique_ptr<WPPkgFs>(new WPPkgFs());
    pkgfs->m_pkgPath = pkgpath;
    idx headerSize   = pkg.Tell();
    for (auto& el : pkgfiles) {
        el.offset += headerSize;
        pkgfs->m_files.insert({ el.path, el });
    }
    return pkgfs;
}

bool WPPkgFs::Contains(std::string_view path) const { return m_files.count(std::string(path)) > 0; }

std::shared_ptr<IBinaryStream> WPPkgFs::Open(std::string_view path) {
    auto pkg = fs::CreateCBinaryStream(m_pkgPath);
    if (! pkg) return nullptr;
    if (Contains(path)) {
        const auto& file = m_files.at(std::string(path));
        return std::make_shared<LimitedBinaryStream>(pkg, file.offset, file.length);
    }
    return nullptr;
}

std::shared_ptr<IBinaryStreamW> WPPkgFs::OpenW(std::string_view) { return nullptr; }
