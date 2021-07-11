#include "Algorism.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Log.h"

using namespace wallpaper;

float algorism::CalculatePersperctiveDistance(float fov, float height) {
	double k = std::tan(glm::radians(fov / 2.0f)) * 2.0f;
	return height / k;
}


float algorism::CalculatePersperctiveFov(float distence, float height) {
	double k = height / distence / 2.0f;
	double angle = std::atan(k) * 2;
	return angle / glm::radians(180.0f) * 180.0f;
}