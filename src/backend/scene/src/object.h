#pragma once
#include <memory>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "common.h"
#include "material.h"
#include "effect.h"
#include "WPRender.h"
#include "GLVertice.h"

namespace wallpaper
{
namespace RenderableType
{
	enum Type {
		Image,
		Particle
	};
};

class RenderObject : public Renderable
{
public:
    RenderObject():m_name(""),m_angles({0.0f,0.0f,0.0f}),m_origin(3),m_scale({1.0f,1.0f,1.0f}),m_visible(true) {};

	
    virtual ~RenderObject() {
        m_vertices.Delete();
    }
    virtual void Load(WPRender&) = 0;
    virtual void Render(WPRender&) = 0;
    virtual bool From_json(const nlohmann::json&);
	virtual RenderableType::Type Type() const = 0;


	const std::string& Name() const {return m_name;};
    const auto& Origin() const {return m_origin;};
    const auto& Angles() const {return m_angles;};
    const auto& Scale() const {return m_scale;};
    auto& Vertices() {return m_vertices;};

    auto* CurVertices() {return m_curVertices;};
    void SetCurVertices(gl::VerticeArray* value) {m_curVertices = value;};

private:
    std::string m_name;
    std::vector<float> m_angles;
    std::vector<float> m_origin;
    std::vector<float> m_scale;
    bool m_visible;
    gl::VerticeArray m_vertices;
    gl::VerticeArray* m_curVertices;
};

class ImageObject : public RenderObject
{
public:
    ImageObject():m_material(*this),size_(2) {};
    bool From_json(const nlohmann::json&);
    ~ImageObject();
    void Load(WPRender&);
    void Render(WPRender&);
	RenderableType::Type Type() const {return RenderableType::Image;};
	bool IsCompose() const;
	const gl::Combos BaseCombos() const {return m_basecombos;};

	gl::GLFramebuffer* CurFbo();
	gl::GLFramebuffer* TargetFbo();
	void SwitchFbo();

private:
	void GenBaseCombos();

	Material m_material;
	std::vector<Effect> effects_;

	gl::Combos m_basecombos;
	gl::Shadervalues shadervalues_;
	gl::VerticeArray m_verticesDefault;

	gl::GLFramebuffer* m_curFbo;
	std::unique_ptr<gl::GLFramebuffer> m_fbo1;
	std::unique_ptr<gl::GLFramebuffer> m_fbo2;

	std::vector<int> size_;
	bool autosize_ = false;
	bool copybackground_ = false;
	float m_alpha = 1.0f;
	float m_brightness  = 1.0f;
	std::vector<float> m_color = {1.0f,1.0f,1.0f};
};

class ParticleObject : public RenderObject
{
public:
    ParticleObject();
    bool From_json(const nlohmann::json&);
    ~ParticleObject() {};
    void Load(WPRender&);
    void Render(WPRender&) {};
	RenderableType::Type Type() const {return RenderableType::Particle;};
};


std::unique_ptr<RenderObject> CreateObject(const nlohmann::json&);
}


