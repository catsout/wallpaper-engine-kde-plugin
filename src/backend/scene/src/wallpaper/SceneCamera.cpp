#include "SceneCamera.h"
#include "SceneNode.h"
#include "Log.h"
#include <iostream>
#include <glm/glm.hpp>
#include "EigenUtil.h"

using namespace wallpaper;
using namespace Eigen;

Matrix4f GetRotationMat(const std::vector<float>& angle) {
	Affine3d trans = Affine3d::Identity();
	trans.prerotate(AngleAxis<double>(angle[1], Vector3d(0.0f, 1.0f, 0.0f))); // y
	trans.prerotate(AngleAxis<double>(angle[0], Vector3d(1.0f, 0.0f, 0.0f))); // x
	trans.prerotate(AngleAxis<double>(-angle[2], Vector3d(0.0f, 0.0f, 1.0f))); // z
	return trans.matrix().cast<float>();
}

Vector3f SceneCamera::GetPosition() const {
	if(m_node) {
		const float * value = &(m_node->Translate())[0];
		return Vector3f(value);
	}
	return Vector3f::Zero();
}

Vector3f SceneCamera::GetDirection() const {
	if(m_node) {
		const auto& angle = m_node->Rotation();
		Matrix4f mat4 = GetRotationMat(angle);
		return (mat4 * Vector4f(0.0f, 0.0f, -1.0f, 0.0f)).head(3);
	}
	return Vector3f(0.0f, 0.0f,-1.0f);
}

Matrix4d SceneCamera::GetViewMatrix() const {
	return m_viewMat;
}

Matrix4d SceneCamera::GetViewProjectionMatrix() const {
	return m_viewProjectionMat;
}

void SceneCamera::CalculateViewProjectionMatrix() {
	// CalculateViewMatrix
	{
		if(m_node) {
			const auto& pos = GetPosition().cast<double>();
			const auto& rmat4 = GetRotationMat(m_node->Rotation());
			Vector3d dir = (rmat4 * Vector4f(0.0f, 0.0f, -1.0f, 0.0f)).head<3>().cast<double>();
			Vector3d up = (rmat4 * Vector4f(0.0f, 1.0f, 0.0f, 0.0f)).head<3>().cast<double>();
			m_viewMat = LookAt(pos, pos+dir, up);
		} else 
			m_viewMat = Matrix4d::Identity();
	};

	if(m_perspective) {
		m_viewProjectionMat = Perspective(Radians(m_fov), m_aspect, m_nearClip, m_farClip) * m_viewMat;
	} else {
		double left = -m_width/2.0f;
		double right = m_width/2.0f;
		double bottom = -m_height/2.0f;
		double up = m_height/2.0f;
		m_viewProjectionMat = Ortho(left, right, bottom, up, m_nearClip, m_farClip) * m_viewMat;
	}
}

void SceneCamera::Update() {
	CalculateViewProjectionMatrix();
}


void SceneCamera::AttatchNode(std::shared_ptr<SceneNode> node) {
	if(!node) {
		LOG_ERROR("Attach a null node to camera");		
		return;
	}
	m_node = node;
	Update();
}