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
	const std::string& GetShader() const {return m_shader;};
	const gl::Shadervalues& GetShadervalues() const {return m_shadervalues;};
	gl::Shadervalues& GetShadervalues() {return m_shadervalues;};
	void SetSize(const std::vector<int>& value) {m_size = value;};

private:
    RenderObject& m_object;
	std::vector<int> m_size;
    bool m_depthtest;
    std::string m_shader;
    std::vector<std::string> textures_;
	gl::Combos m_combos;
	std::string m_constShadervalues;
	gl::Shadervalues m_shadervalues;
	std::vector<gl::GLTexture> glTextrues_;
};

}
