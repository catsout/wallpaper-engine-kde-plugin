#include "../../WPPkgFs.h"
#include "../../Fs/PhysicalFs.h"
#include "../../Fs/VFS.h"
#include "../../Utils/Log.h"

using namespace wallpaper::fs;

int main(int argc, char** argv) {
	if(argc != 3) return 1;
	auto pfs = CreatePhysicalFs(argv[1]);
	auto fs = WPPkgFs::CreatePkgFs(argv[2]);
	VFS vfs {};
	vfs.Mount("/assets", std::move(fs));
	vfs.Mount("/assets", std::move(pfs));
	auto file = vfs.Open("/assets/scene.json");
	auto file2 = vfs.Open("/assets/shaders/genericimage2.frag");
	if(file) {
		LOG_INFO(file->ReadAllStr());
	}
	if(file2) {
		LOG_INFO(file2->ReadAllStr());
	}
	return 0;
}