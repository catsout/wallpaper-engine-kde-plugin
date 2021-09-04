#include "WPPkgFs.h"
#include "Utils/Logging.h"
#include "Fs/LimitedBinaryStream.h"
#include <vector>

using namespace wallpaper::fs;

static std::string ReadSizedString(IBinaryStream& f) {
	uint32_t len = f.ReadUint32();
	std::string result;
	result.resize(len);
	f.Read(result.data(), len);
	return result;
}

std::unique_ptr<WPPkgFs> WPPkgFs::CreatePkgFs(std::string_view pkgpath) {
	auto ppkg = fs::CreateCBinaryStream(pkgpath);
	if(!ppkg) return nullptr;

	auto& pkg = *ppkg;
   	std::string ver = ReadSizedString(pkg);
	LOG_INFO("pkg version: %s", ver.data());

	std::vector<PkgFile> pkgfiles;
    int entryCount = pkg.ReadInt32();
    for(int i=0 ; i<entryCount ; i++) {
        std::string path = "/" + ReadSizedString(pkg);
        uint32_t offset = pkg.ReadUint32();
        uint32_t length = pkg.ReadUint32();
		pkgfiles.push_back({path, offset, length});
    }
	auto pkgfs = std::unique_ptr<WPPkgFs>(new WPPkgFs());
	pkgfs->m_pkgPath = pkgpath;
	uint32_t headerSize = pkg.Tell();
	for(auto& el:pkgfiles) {
		el.offset += headerSize;
		pkgfs->m_files.insert({el.path, el});
	}
	return pkgfs;
}


bool WPPkgFs::Contains(std::string_view path) const {
	return m_files.count(std::string(path)) > 0;
}

std::shared_ptr<IBinaryStream> WPPkgFs::Open(std::string_view path) {
	auto pkg = fs::CreateCBinaryStream(m_pkgPath);
	if(!pkg) return nullptr;
	if(Contains(path)) {
		const auto& file = m_files.at(std::string(path));
		return std::make_shared<LimitedBinaryStream>(pkg,file.offset,file.length);
	} 
	return nullptr;
}