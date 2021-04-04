#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include "pkg.h"
#include "common.h"
//#include "object.h"
//#include "WPRender.h"
#include "FpsCounter.h"
#include "FrameTimer.h"
#include "SceneMesh.h"

#include "Scene.h"
#include "WPSceneParser.h"
#include "GLGraphicManager.h"

namespace wallpaper
{
class WallpaperGL
{
public:
    static const fs::file_node& GetPkgfs(){
        return m_pkgfs;
    }
    WallpaperGL():m_fboTrans(1.0f),m_mousePos({0,0}),m_aspect(16.0f/9.0f) {};
    ~WallpaperGL() { Clear(); };
	bool Init(void *get_proc_address(const char *));
    void Load(const std::string& pkg_path);
    void Render(uint fbo, int width, int height);
    void Clear();
	void SetAssets(const std::string& path);
    void SetObjEffNum(int obj, int eff);
	void SetFlip(bool value) {m_flip = value;};
	void SetMousePos(float x, float y) {m_mousePos = std::vector<float>({x,y});};
	uint32_t CurrentFps() const {return m_fpsCounter.Fps();};
	uint8_t Fps() const {return m_frameTimer.Fps();}
	void SetFps(uint8_t value) {m_frameTimer.SetFps(value);}
	void SetUpdateCallback(const std::function<void()>&);
    static int ObjNum() {return m_objnum;};
    static int EffNum() {return m_effnum;};

	bool Loaded() const {return m_loaded;};

	void Start();
	void Stop();

private:
    static fs::file_node m_pkgfs;
	std::string m_pkgPath;
	glm::mat4 m_fboTrans;

	bool m_inited = false;
	bool m_loaded = false;
	int m_framecount = 0;
	bool m_flip = false;
	std::vector<float> m_mousePos;

	SceneMesh m_mesh;

	FpsCounter m_fpsCounter;
	FrameTimer m_frameTimer;

	float m_aspect;

    // for debug
    static int m_objnum;
    static int m_effnum;

	WPSceneParser m_parser;
	GLGraphicManager m_gm;


	std::unique_ptr<Scene> m_scene;
};

}
