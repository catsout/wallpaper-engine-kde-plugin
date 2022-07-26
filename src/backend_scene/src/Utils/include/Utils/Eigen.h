#pragma once
#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace Eigen {

double inline Radians(double a) {
	static const double pi {std::atan(1)*4};
	return (a/180.0f)*pi;
}

Matrix4d inline LookAt(Vector3d eye, Vector3d center, Vector3d up) {
	Vector3d camDir = center - eye;
	// cam look at neg z
	Vector3d zAxis = -camDir.normalized();
	Vector3d xAxis = up.cross(zAxis).normalized();
	Vector3d yAxis = zAxis.cross(xAxis).normalized();

	Affine3d trans = Affine3d::Identity();
	// base change
	trans.linear().col(0) = xAxis;
	trans.linear().col(1) = yAxis;
	trans.linear().col(2) = zAxis;

	// translate
	trans *= Translation3d(-eye);

	return trans.matrix();
}

Matrix4d inline Ortho(double left, double right, double bottom, double top, double nearz, double farz) {
	// translate to orgin + scale to [-1,1]
	Affine3d trans = Affine3d::Identity();
	trans.pretranslate(Vector3d(
		-(left+right)/2.0f,
		-(top+bottom)/2.0f,
		-(nearz+farz)/2.0f
	));
	trans.prescale(Vector3d(
		2.0f/(right-left),
		2.0f/(top-bottom),
		2.0f/(farz-nearz)
	));
	// look at neg z, switch z before
	trans.scale(Vector3d(1.0f,1.0f,-1.0f));
	
	// to [0, 1]
	trans.prescale(Vector3d(
		1.0f, 1.0f, 0.5f
	));
	trans.pretranslate(Vector3d(0.0f, 0.0f, 0.5f));
	return trans.matrix();
}

Matrix4d inline Perspective(double fov, double aspect, double nearz, double farz) {
	Projective3d trans = Projective3d::Identity();
	// no need to deal with neg z
	trans.prescale(Vector3d(nearz, nearz, (nearz + farz)));
	trans(3,2) = 1.0f;
	trans(3,3) = 0.0f;
	trans(2,3) = -nearz*farz;
	double top = std::tan(fov/2.0f) * std::abs(nearz);
	double right = top * aspect;
	// as look at neg z, switch z before
	trans.scale(Vector3d(1.0f, 1.0f, -1.0f));
	// as ortho also switch, do switch after to cancle 
	trans.prescale(Vector3d(1.0f, 1.0f, -1.0f));
	return Ortho(-right, right, -top, top, nearz, farz) * trans.matrix();
}
}