#pragma once
#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace Eigen {

double inline Radians(double a) {
	static const double pi {std::atan(1)*4};
	return (a/180.0f)*pi;
}

Matrix4d LookAt(Vector3d eye, Vector3d center, Vector3d up) {
	Vector3d camDir = center - eye;
	Vector3d zAxis = -camDir.normalized();
	Vector3d xAxis = up.cross(zAxis).normalized();
	Vector3d yAxis = zAxis.cross(xAxis).normalized();

	Affine3d trans = Affine3d::Identity();
	// base change
	trans.linear().col(0) = xAxis;
	trans.linear().col(1) = yAxis;
	trans.linear().col(2) = zAxis;

	// translate
	trans.pretranslate(-Vector3d(xAxis.dot(eye),yAxis.dot(eye),zAxis.dot(eye)));
	return trans.matrix();
}

Matrix4d Ortho(double left, double right, double bottom, double top, double nearz, double farz) {
	// translate to orgin + scale to [-1,1]
	Affine3d trans = Affine3d::Identity();
	trans.pretranslate(Vector3d(
		-(left+right)/2.0f,
		-(top+bottom)/2.0f,
		(nearz+farz)/2.0f
	));
	trans.prescale(Vector3d(
		2.0f/(right-left),
		2.0f/(top-bottom),
		-2.0f/(farz-nearz)
	));
	return trans.matrix();
}

Matrix4d Perspective(double fov, double aspect, double nearz, double farz) {
	Projective3d trans = Projective3d::Identity();
	// as z is negtive, keep w is positive
	trans.prescale(Vector3d(nearz, nearz, (nearz + farz)));
	trans(3,2) = -1.0f;
	trans(3,3) = 0.0f;
	trans(2,3) = nearz*farz;
	double top = std::tan(fov/2.0f) * nearz;
	double right = top * aspect;
	return Ortho(-right, right, -top, top, nearz, farz) * trans.matrix();
}
}