#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include "Type.h"
#include "pkg.h"

namespace wallpaper
{
class WallpaperGL
{
public:
    static const fs::file_node& GetPkgfs(){
        return m_pkgfs;
    }
    WallpaperGL();
    ~WallpaperGL();
	bool Init(void *get_proc_address(const char *));
    void Load(const std::string& pkg_path);
    void Render();//uint fbo, int width, int height);
    void Clear();
	void SetAssets(const std::string& path);
	void SetFlip(bool value) {m_flip = value;};
	void SetMousePos(float x, float y) {m_mousePos = std::vector<float>({x,y});};

	// call this after loaded
	void SetDefaultFbo(uint fbo, uint32_t w, uint32_t h);
	void SetFillMode(FillMode);

	uint32_t CurrentFps() const;
	uint8_t Fps() const;
	void SetFps(uint8_t value);
	void SetUpdateCallback(const std::function<void()>&);

	bool Loaded() const {return m_loaded;};

	void Start();
	void Stop();

private:
	class impl;
    std::unique_ptr<impl> pImpl;

    static fs::file_node m_pkgfs;
	std::string m_pkgPath;

	bool m_inited {false};
	bool m_loaded {false};
	int m_framecount {0};
	bool m_flip {false};
	std::vector<float> m_mousePos;

	float m_aspect;
	FillMode m_fillMode {FillMode::ASPECTCROP};
};

}
