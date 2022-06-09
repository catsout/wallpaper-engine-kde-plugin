#include "SceneNode.h"

#include <Eigen/Geometry>

using namespace wallpaper;
using namespace Eigen;

Matrix4d SceneNode::GetLocalTrans() const {
	Affine3d trans = Affine3d::Identity();
	trans.prescale(m_scale.cast<double>());


	trans.prerotate(AngleAxis<double>(m_rotation.x(), Vector3d::UnitX())); // x
	trans.prerotate(AngleAxis<double>(m_rotation.y(), Vector3d::UnitY())); // y
	trans.prerotate(AngleAxis<double>(m_rotation.z(), Vector3d::UnitZ())); // z

	trans.pretranslate(m_translate.cast<double>());


	return trans.matrix();
}
