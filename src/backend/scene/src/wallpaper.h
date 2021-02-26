#pragma once
#include <memory>
#include <vector>
#include "pkg.h"
#include "object.h"
#include "WPRender.h"
#include "GLVertice.h"

namespace wallpaper
{

class WallpaperGL
{
public:
    static const fs::file_node& GetPkgfs(){
        return m_pkgfs;
    }
	bool Init(void *get_proc_address(const char *));
    void Load(const std::string& pkg_path);
    void Render(uint fbo, int width, int height);
    void Clear();
	void SetAssets(const std::string& path);
private:
    static fs::file_node m_pkgfs;
	std::string m_pkgPath;
    std::vector<std::unique_ptr<RenderObject>> m_objects;
	gl::VerticeArray vertices_;
	WPRender wpRender_;
	bool m_inited = false;
	int framecount_ = 0;
};

}
