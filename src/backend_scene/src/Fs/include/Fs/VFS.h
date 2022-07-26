#pragma once
#include <vector>
#include <memory>
#include <string>
#include <tuple>
#include <algorithm>
#include "Fs.h"
#include "Utils/Logging.h"
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{
namespace  fs
{

/*
 * path like: /root/dir1/dir2/file
 * not resolve "." "..", empty dir name as this using full path match
 */

class VFS : NoCopy,NoMove {
public:
	struct MountedFs {
		std::string name;
		std::string mountPoint; // full path of mount point in "/", last char can't be "/"
		std::unique_ptr<Fs> fs;
		static bool CheckMountPoint(const std::string_view mountPoint) {
			return mountPoint[mountPoint.size()-1] != '/';
		}
		static bool InMountPoint(const std::string_view mountPoint, const std::string_view path) {
			return path.compare(0, mountPoint.size() + 1, std::string(mountPoint) + '/') == 0;
		}
		static std::string GetPathInMount(const std::string_view mountPoint, const std::string_view path) {
			return std::string(path.substr(mountPoint.size()));
		}
	};
public:
	VFS() = default;
	~VFS() = default;

	bool Mount(std::string_view mountpoint, std::unique_ptr<Fs> fs, std::string_view name="") {
		if(!MountedFs::CheckMountPoint(mountpoint) || !fs) return false;

		m_mountedFss.push_back({std::string(name), std::string(mountpoint), std::move(fs)});
		return true;
	}
	bool Unmount(std::string_view mountpoint) {
		for(auto iter = m_mountedFss.rbegin();iter < m_mountedFss.rend();iter++) {
			if(iter->mountPoint == mountpoint) {
				m_mountedFss.erase((++iter).base());
				return true;
			}
		}
		LOG_INFO("mount point not exist");
		return false;
	}
	bool IsMounted(std::string_view name) {
		for(const auto& el:m_mountedFss) {
			if(el.name == name)
				return true;
		}
		return false;
	}
	std::shared_ptr<IBinaryStream> Open(std::string_view path) {
        auto find_it = std::find_if(m_mountedFss.rbegin(), m_mountedFss.rend(), [&path](const auto& mfs) {
			return MountedFs::InMountPoint(mfs.mountPoint, path) && 
                   mfs.fs->Contains(MountedFs::GetPathInMount(mfs.mountPoint, path));
        });
        if(find_it != std::rend(m_mountedFss)) 
            return find_it->fs->Open(MountedFs::GetPathInMount(find_it->mountPoint, path));
		LOG_ERROR("not found \"%s\" in vfs", path.data());
		return nullptr;
	}
	std::shared_ptr<IBinaryStreamW> OpenW(std::string_view path) {
        auto find_it = std::find_if(m_mountedFss.rbegin(), m_mountedFss.rend(), [&path](const auto& mfs) {
			return MountedFs::InMountPoint(mfs.mountPoint, path) && 
                   mfs.fs->Contains(MountedFs::GetPathInMount(mfs.mountPoint, path));
        });
        if (find_it == std::rend(m_mountedFss)) {
            find_it = std::find_if(m_mountedFss.rbegin(), m_mountedFss.rend(), [&path](const auto& mfs) {
			    return MountedFs::InMountPoint(mfs.mountPoint, path);
            });
        }
        if(find_it != std::rend(m_mountedFss))
            return find_it->fs->OpenW(MountedFs::GetPathInMount(find_it->mountPoint, path));
		LOG_ERROR("not found \"%s\" in vfs", path.data());
		return nullptr;
	}
	bool Contains(std::string_view path) const {
		for(auto iter = m_mountedFss.rbegin();iter < m_mountedFss.rend();iter++) {
			auto& el = *iter;
			if(MountedFs::InMountPoint(el.mountPoint, path)) {
				auto mpath = MountedFs::GetPathInMount(el.mountPoint, path);
				if(el.fs->Contains(mpath))
					return true;
			}
		}
		return false;
	}
private:
	std::vector<MountedFs> m_mountedFss;
};

inline std::string GetFileContent(fs::VFS& vfs, std::string_view path) {
	auto f = vfs.Open(path);
	if(f) return f->ReadAllStr();	
	return "";
} 

}
}
