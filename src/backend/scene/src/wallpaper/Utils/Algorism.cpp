#include "Algorism.h"
#include "Eigen.h"

using namespace wallpaper;
using namespace Eigen;

float algorism::CalculatePersperctiveDistance(float fov, float height) {
	double k = std::tan(Radians(fov / 2.0f)) * 2.0f;
	return height / k;
}


float algorism::CalculatePersperctiveFov(float distence, float height) {
	double k = height / distence / 2.0f;
	double angle = std::atan(k) * 2;
	return angle / Radians(180.0f) * 180.0f;
}
