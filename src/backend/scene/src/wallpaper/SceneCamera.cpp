#include "SceneCamera.h"
#include "SceneNode.h"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace wallpaper;

glm::vec3 SceneCamera::GetPosition() const {
	const float * value = &(m_node->Translate())[0];
	return glm::make_vec3(value);
}

glm::vec3 SceneCamera::GetDirection() const {
	const auto& angle = m_node->Rotation();
	glm::mat4 mat4(1.0f);
	mat4 = glm::rotate(glm::mat4(1.0f), -angle[2], glm::vec3(0,0,1));
	mat4 = glm::rotate(mat4, angle[0], glm::vec3(1,0,0)); // x
	mat4 = glm::rotate(mat4, angle[1], glm::vec3(0,1,0)); // y
	return mat4 * glm::vec4(0, 0, -1.0f, 0);
}

glm::mat4 GetRotationMat(const std::vector<float>& angle) {
	glm::mat4 mat4(1.0f);
	mat4 = glm::rotate(glm::mat4(1.0f), -angle[2], glm::vec3(0,0,1));
	mat4 = glm::rotate(mat4, angle[0], glm::vec3(1,0,0)); // x
	mat4 = glm::rotate(mat4, angle[1], glm::vec3(0,1,0)); // y
	return mat4;
}

glm::mat4 SceneCamera::GetViewMatrix() const {
	const auto& pos = GetPosition();
	const auto& rmat4 = GetRotationMat(m_node->Rotation());
	glm::vec3 dir = rmat4 * glm::vec4(0, 0, -1.0f, 0);
	glm::vec3 up = rmat4 * glm::vec4(0, 1.0f, 0, 0);
	return glm::lookAt(pos, pos+dir, up);
}

glm::mat4 SceneCamera::GetProjectionMatrix() const {
	float left = -m_width/2.0f;
	float right = m_width/2.0f;
	float bottom = -m_height/2.0f;
	float up = m_height/2.0f;
	return glm::ortho(left, right, bottom, up, m_nearClip, m_farClip);
}
