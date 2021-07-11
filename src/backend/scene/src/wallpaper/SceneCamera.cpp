#include "SceneCamera.h"
#include "SceneNode.h"
#include "Log.h"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace wallpaper;

glm::mat4 GetRotationMat(const std::vector<float>& angle) {
	glm::mat4 mat4(1.0f);
	mat4 = glm::rotate(glm::mat4(1.0f), -angle[2], glm::vec3(0,0,1)); // z
	mat4 = glm::rotate(mat4, angle[0], glm::vec3(1,0,0)); // x
	mat4 = glm::rotate(mat4, angle[1], glm::vec3(0,1,0)); // y
	return mat4;
}

glm::vec3 SceneCamera::GetPosition() const {
	if(m_node) {
		const float * value = &(m_node->Translate())[0];
		return glm::make_vec3(value);
	}
	return glm::vec3(0,0,0);
}

glm::vec3 SceneCamera::GetDirection() const {
	if(m_node) {
		const auto& angle = m_node->Rotation();
		auto mat4 = GetRotationMat(angle);
		return mat4 * glm::vec4(0, 0, -1.0f, 0);
	}
	return glm::vec3(0, -1.0f, 0);
}

glm::mat4 SceneCamera::GetViewMatrix() const {
	return m_viewMat;
}

glm::mat4 SceneCamera::GetViewProjectionMatrix() const {
	return m_viewProjectionMat;
}

void SceneCamera::CalculateViewProjectionMatrix() {
	// CalculateViewMatrix
	{
		if(m_node) {
			const auto& pos = GetPosition();
			const auto& rmat4 = GetRotationMat(m_node->Rotation());
			glm::vec3 dir = rmat4 * glm::vec4(0, 0, -1.0f, 0);
			glm::vec3 up = rmat4 * glm::vec4(0, 1.0f, 0, 0);
			m_viewMat = glm::lookAt(pos, pos+dir, up);
		} else 
			m_viewMat = glm::mat4(1.0f);
	};

	if(m_perspective) {
		m_viewProjectionMat = glm::perspective(glm::radians(m_fov), m_aspect, m_nearClip, m_farClip) * m_viewMat;
	} else {
		float left = -m_width/2.0f;
		float right = m_width/2.0f;
		float bottom = -m_height/2.0f;
		float up = m_height/2.0f;
		m_viewProjectionMat = glm::ortho(left, right, bottom, up, m_nearClip, m_farClip) * m_viewMat;
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