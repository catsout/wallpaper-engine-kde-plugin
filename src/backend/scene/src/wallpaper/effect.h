#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "WPRender.h"
#include "material.h"
#include "GLWrapper.h"
#include "WPJson.h"
#include "SceneMesh.h"

namespace wallpaper
{
class RenderObject;
class ImageObject;

struct BindInfo {
	std::string name;
	int index;
};

class MaterialData {
public:
	MaterialData(RenderObject& obj, const std::vector<int>& size):material(obj, size) {};
	~MaterialData() {};
	Material material;
	std::string target;
	std::vector<BindInfo> bindInfos;
};

class FboData {
public:
	FboData(float scale):scale(scale) {};
	FboData():scale(1.0f) {};
	std::unique_ptr<gl::GLFramebuffer> fbo;
	float scale;
};

class Effect : public Renderable {
public:
    Effect(ImageObject& img,std::vector<int> size):m_imgObject(img),m_size(size),m_visible(true) {};
    ~Effect() {};
    Effect(const Effect&) = delete;
    Effect& operator=(const Effect&) = delete;

    Effect(Effect&& o):m_imgObject(o.m_imgObject),
		m_name(o.m_name),
		m_size(std::move(o.m_size)),
		m_materials(std::move(o.m_materials)),
		m_fboDataMap(std::move(o.m_fboDataMap)) {};

    bool From_json(const nlohmann::json&);
    void Load(WPRender&);
    void Render(WPRender&);

	bool Visible() const {return m_visible;};

    const std::string& Name() {return m_name;};
private:
    ImageObject& m_imgObject;
	std::string m_name;
	std::vector<int> m_size;
	std::vector<MaterialData> m_materials;
	std::unordered_map<std::string, FboData> m_fboDataMap;
	
	SceneMesh m_mesh_normal;
	
	bool m_visible;
};
}
