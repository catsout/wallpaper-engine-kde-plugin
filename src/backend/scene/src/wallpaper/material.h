#pragma once
#include <unordered_map>
#include <memory.h>
#include "common.h"
#include "WPRender.h"
#include "WPJson.h"
#include "SceneMesh.h"

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

namespace Blending {
	enum Type {
		normal,
		translucent,
		additive
	};
	void ApplayBlending(Blending::Type);
}

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
	const auto& GetTexture(int index) {return m_textures.at(index);};
	std::vector<std::string>& GetTextures() {return m_textures;};
	void SetSize(const std::vector<int>& value) {m_size = value;};
	const Blending::Type& Blending() const {return m_blending;};
	void SetMesh(const SceneMesh* mesh) {m_mesh = mesh;};

private:
    RenderObject& m_object;
	std::vector<int> m_size;
	Blending::Type m_blending;
    bool m_depthtest;
    std::string m_shader;
    std::vector<std::string> m_textures;
	gl::Combos m_combos;
	std::string m_constShadervalues;
	// shader value only include the shader specific uniform
	gl::Shadervalues m_shadervalues;
//	std::vector<gl::GLTexture> m_glTextrues;

    const SceneMesh* m_mesh;
};

}
