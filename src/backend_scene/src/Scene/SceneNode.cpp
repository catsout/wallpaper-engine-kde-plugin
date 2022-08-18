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

void SceneNode::UpdateTrans() {
    if (! m_dirty) return;
    m_dirty = false;

    if (m_parent) {
        m_parent->UpdateTrans();
    }
    {
        Affine3d trans = Affine3d::Identity();
        if (m_parent) {
            trans *= m_parent->ModelTrans();
        }
        m_trans = (trans * GetLocalTrans()).matrix();
    }
}

void SceneNode::MarkTransDirty() {
    if (! m_dirty) {
        m_dirty = true;
        for (auto& child : m_children) {
            child->MarkTransDirty();
        }
    }
}
