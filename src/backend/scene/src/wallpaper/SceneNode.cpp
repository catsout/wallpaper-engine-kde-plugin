#include "SceneNode.h"
#include "common.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace wallpaper;

glm::mat4 SceneNode::GetLocalTrans() const {
	glm::mat4 mat4(1.0f);
	mat4 = glm::translate(glm::mat4(1.0f), glm::make_vec3(&m_translate[0])); 
	//2. rotation
	mat4 = glm::rotate(mat4, -m_rotation[2], glm::vec3(0,0,1)); // z, need negative
	mat4 = glm::rotate(mat4, m_rotation[0], glm::vec3(1,0,0)); // x
	mat4 = glm::rotate(mat4, m_rotation[1], glm::vec3(0,1,0)); // y
	//1. scale
	mat4 = glm::scale(mat4, glm::make_vec3(&m_scale[0]));
	return mat4;
}
