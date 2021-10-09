#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include "Type.h"

namespace wallpaper
{
class WallpaperGL
{
public:
    WallpaperGL();
    ~WallpaperGL();
	WallpaperGL(const WallpaperGL&) = delete;
	WallpaperGL(WallpaperGL&&) = delete;
	WallpaperGL& operator=(WallpaperGL&) = delete;
	WallpaperGL& operator=(WallpaperGL&&) = delete;

	bool Init(void *get_proc_address(const char *));

    void LoadPkgFile(const std::string& path) {
		Load(path, true);
	}
    void LoadPkgDir(const std::string& path) {
		Load(path, false);
	}
    void Render();//uint fbo, int width, int height);
    void Clear();
	void SetAssets(const std::string& path);
	void SetFlip(bool value);
	void SetMousePos(float x, float y) {m_mousePos = std::vector<float>({x,y});};

	// call this after loaded
	void SetDefaultFbo(uint fbo, uint16_t w, uint16_t h);
	void SetFillMode(FillMode);

	uint32_t CurrentFps() const;
	uint8_t Fps() const;
	void SetFps(uint8_t value);
	void SetUpdateCallback(const std::function<void()>&);

	bool Loaded() const {return m_loaded;};

	void Start();
	void Stop();
	bool IsRunning() const;

	void OutGraphviz(const std::string& path);

	float Volume() const;
	bool Muted() const;
	void SetMuted(bool);
	void SetVolume(float);

private:
    void Load(const std::string& path, bool ispkgfile=true);
private:
	class impl;
    std::unique_ptr<impl> pImpl;

	std::string m_pkgPath;
	std::string m_assetsPath;

	bool m_inited {false};
	bool m_loaded {false};
	int m_framecount {0};
	std::vector<float> m_mousePos;

	float m_aspect;
	FillMode m_fillMode {FillMode::ASPECTCROP};
};

}
