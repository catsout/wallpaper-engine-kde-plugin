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

	gl::GLFramebuffer* CurFbo() {return m_curFbo;};
	void SetCurFbo(gl::GLFramebuffer* value) {m_curFbo=value;};
    const std::string& Name() const {return m_name;};
    const auto& Origin() const {return m_origin;};
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
	gl::GLFramebuffer* m_curFbo;
};

class ImageObject : public RenderObject
{
public:
    ImageObject():m_material(*this) {};
    bool From_json(const nlohmann::json&);
    ~ImageObject();
    void Load(WPRender&);
    void Render(WPRender&);

private:
	std::vector<int> size_;
    Material m_material;
	std::vector<Effect> effects_;
	std::unique_ptr<gl::GLFramebuffer> fbo_;
	gl::Shadervalues shadervalues_;
	bool autosize_ = false;
	bool copybackground_ = false;
	float alpha_ = 1.0f;
	std::vector<float> color_ = {1.0f,1.0f,1.0f};
    gl::VerticeArray m_verticesDefault;
};

class ParticleObject : public RenderObject
{
public:
    ParticleObject();
    bool From_json(const nlohmann::json&);
    ~ParticleObject() {};
    void Load(WPRender&);
    void Render(WPRender&) {};
};


std::unique_ptr<RenderObject> CreateObject(const nlohmann::json&);
}


