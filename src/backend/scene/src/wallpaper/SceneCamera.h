#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "SceneImageEffectLayer.h"

namespace wallpaper
{

class SceneNode;

class SceneCamera {
public:
	explicit SceneCamera(int32_t width, int32_t height, float near, float far)
		: m_width(width),
		  m_height(height),
		  m_nearClip(near),
		  m_farClip(far),
		  m_perspective(false) {}

	explicit SceneCamera(float aspect, float near, float far, float fov)
		: m_width(aspect),
		  m_height(1.0f),
		  m_nearClip(near),
		  m_farClip(far),
		  m_fov(fov),
		  m_perspective(true) {}

	SceneCamera(const SceneCamera&) = default;

	void AttatchNode(SceneNode* node) { m_node = node; }

	bool Perspective() const { return m_perspective; }
	float Aspect() const { return m_width/m_height; }
	float Width() const { return m_width; }
	float Height() const { return m_height; }
	float NearClip() const { return m_nearClip; }
	float FarClip() const { return m_farClip; }
	float Fov() const { return m_fov; }

	void AttatchImgEffect(const std::shared_ptr<SceneImageEffectLayer>& eff) { m_imgEffect = eff; }
	bool HasImgEffect() const { return (bool)m_imgEffect; }
	auto& GetImgEffect() { return m_imgEffect; }

	glm::vec3 GetPosition() const;	
	glm::vec3 GetDirection() const;
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;

	SceneNode* GetAttachedNode() const { return m_node; }

private:
	float m_width;
	float m_height;
	float m_nearClip;
	float m_farClip;
	float m_fov;
	bool m_perspective;

	SceneNode* m_node;
	std::shared_ptr<SceneImageEffectLayer> m_imgEffect {nullptr};
};
}
