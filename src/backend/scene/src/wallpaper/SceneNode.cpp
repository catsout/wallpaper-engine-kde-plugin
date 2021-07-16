#include "SceneNode.h"

#include <Eigen/Geometry>

using namespace wallpaper;
using namespace Eigen;

Matrix4d SceneNode::GetLocalTrans() const {
	Affine3d trans = Affine3d::Identity();
	trans.prescale(Vector3f(&m_scale[0]).cast<double>());

	trans.prerotate(AngleAxis<double>(m_rotation[1], Vector3d::UnitY())); // y
	trans.prerotate(AngleAxis<double>(m_rotation[0], Vector3d::UnitX())); // x
	trans.prerotate(AngleAxis<double>(-m_rotation[2], Vector3d::UnitZ())); // z

	trans.pretranslate(Vector3f(&m_translate[0]).cast<double>());


	return trans.matrix();
}
