#pragma once
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <memory.h>
#include "common.h"
#include "WPRender.h"
#include "GLVertice.h"

namespace wallpaper
{
/*
{
	"passes" :
	[
		{
			"blending" : "translucent",
			"cullmode" : "nocull",
			"depthtest" : "disabled",
			"depthwrite" : "disabled",
			"shader" : "",
			"textures" : [ "" ]
		}
	]
}
*/

class RenderObject;

class Material : public Renderable
{
public:
    Material(RenderObject& object, const std::vector<int>& size);
    Material(RenderObject& object):Material(object, {1920,1080}) {};
    bool From_json(const nlohmann::json&);
    ~Material(){};
    void Load(WPRender&);
    void Render(WPRender&);
	const std::string& GetShader() const {return shader_;};
	const gl::Shadervalues& GetShadervalues() const {return shadervalues_;};
	gl::Shadervalues& GetShadervalues() {return shadervalues_;};
	void SetSize(const std::vector<int>& value) {size_ = value;};

private:
    RenderObject& object_;
	std::vector<int> size_;
    bool depthtest_;
    std::string shader_;
    std::vector<std::string> textures_;
	gl::Combos combos_;
	std::string constShadervalues_;
	gl::Shadervalues shadervalues_;
	std::vector<gl::GLTexture> glTextrues_;
};

}
