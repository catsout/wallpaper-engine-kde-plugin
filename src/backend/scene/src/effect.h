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
	MaterialData(RenderObject& obj, std::vector<int> size):material(obj, size) {};
	~MaterialData() {};
	Material material;
	std::string target;
	std::vector<BindInfo> bindInfos;
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
		fboMap_(std::move(o.fboMap_)) {};

    bool From_json(const nlohmann::json&);
    void Load(WPRender&);
    void Render(WPRender&);

    const std::string& Name() {return name_;};
private:
    ImageObject& imgObject_;
	std::string name_;
	std::vector<int> size_;
	std::vector<MaterialData> materials_;
	std::unordered_map<std::string, std::unique_ptr<gl::GLFramebuffer>> fboMap_;
    gl::VerticeArray vertices_;
//	gl::GLFramebuffer* fboPre_;
};
}
