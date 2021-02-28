#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "WPRender.h"
#include "material.h"
#include "GLWrapper.h"


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
    Effect(ImageObject& img,std::vector<int> size):imgObject_(img),size_(size) {};
    ~Effect() {};
    Effect(const Effect&) = delete;
    Effect& operator=(const Effect&) = delete;

    Effect(Effect&& o):imgObject_(o.imgObject_),
		name_(o.name_),
		size_(std::move(o.size_)),
		materials_(std::move(o.materials_)),
		fboDataMap_(std::move(o.fboDataMap_)) {};

    bool From_json(const nlohmann::json&);
    void Load(WPRender&);
    void Render(WPRender&);

    const std::string& Name() {return name_;};
private:
    ImageObject& imgObject_;
	std::string name_;
	std::vector<int> size_;
	std::vector<MaterialData> materials_;
	std::unordered_map<std::string, FboData> fboDataMap_;
    gl::VerticeArray vertices_;
};
}
