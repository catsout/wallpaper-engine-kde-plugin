#pragma once
#include <list>
#include <vector>
#include <memory>
#include <Eigen/Dense>
#include "SceneMesh.h"
#include "SceneCamera.h"

namespace wallpaper
{

class SceneNode {
public:
	SceneNode():m_translate(3),m_scale{1.0f,1.0f,1.0f},m_rotation(3),m_name() {}
	SceneNode(const std::vector<float>& translate, const std::vector<float>& scale, const std::vector<float>& rotation, const std::string& name="")
		: m_translate(translate),
		  m_scale(scale),
		  m_rotation(rotation),
		  m_name(name) {};

	const auto& Camera() const { return m_cameraName; }
	void SetCamera(const std::string& name) { m_cameraName = name; }
	void AddMesh(std::shared_ptr<SceneMesh> mesh) { m_mesh = mesh; }
	void AppendChild(std::shared_ptr<SceneNode> sub) {
		sub->m_parent = this;
		m_children.push_back(sub);
	}
	Eigen::Matrix4d GetLocalTrans() const;

	const auto& Translate() const { return m_translate; }
	const auto& Rotation() const { return m_rotation; }
	void SetTranslate(const std::vector<float>& value) { m_translate = value; }

	void CopyTrans(const SceneNode& node) {
		m_translate = node.m_translate;
		m_scale = node.m_scale;
		m_rotation = node.m_rotation;
	}
	
	SceneMesh* Mesh() { return m_mesh.get(); }
	
	const auto& GetChildren() const { return m_children; }
	auto& GetChildren() { return m_children; }
private:
	std::string m_name;
	Eigen::Matrix4d m_trans;
	std::vector<float> m_translate;
	std::vector<float> m_scale;
	std::vector<float> m_rotation;

	std::shared_ptr<SceneMesh> m_mesh;

	// specific a camera not active, used for image effect
	std::string m_cameraName; 

	SceneNode* m_parent;
	std::list<std::shared_ptr<SceneNode>> m_children;
};
}
