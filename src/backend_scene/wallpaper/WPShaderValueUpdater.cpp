#include "WPShaderValueUpdater.hpp"
#include "Scene/Scene.h"
#include "SpriteAnimation.hpp"
#include "SpecTexs.hpp"
#include "Utils/ArrayHelper.hpp"

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>
#include <chrono>
#include <ctime>

using namespace wallpaper;
using namespace Eigen;

void WPShaderValueUpdater::FrameBegin() {
    /*
        using namespace std::chrono;
        auto nowTime = system_clock::to_time_t(system_clock::now());
        auto cTime   = std::localtime(&nowTime);
        m_dayTime =
            (((cTime->tm_hour * 60) + cTime->tm_min) * 60 + cTime->tm_sec) / (24.0f * 60.0f
       * 60.0f);
    */
}

void WPShaderValueUpdater::FrameEnd() {}

void WPShaderValueUpdater::MouseInput(double x, double y) {
    m_mousePos[0] = x;
    m_mousePos[1] = y;
}

void WPShaderValueUpdater::UpdateUniforms(SceneNode* pNode, sprite_map_t& sprites,
                                          const ExistsUniformOp& existsOp,
                                          const UpdateUniformOp& updateOp) {
    if (! pNode->Mesh()) return;

    const SceneCamera* camera;
    std::string_view   cam_name = pNode->Camera();
    if (! pNode->Camera().empty()) {
        camera = m_scene->cameras.at(cam_name.data()).get();
    } else
        camera = m_scene->activeCamera;

    if (! camera) return;

    auto* material = pNode->Mesh()->Material();
    if (! material) return;
    // auto& shadervs = material->customShader.updateValueList;
    // const auto& valueSet = material->customShader.valueSet;

    bool hasNodeData = exists(m_nodeDataMap, pNode);
    if (hasNodeData) {
        auto& nodeData = m_nodeDataMap.at(pNode);
        for (const auto& el : nodeData.renderTargets) {
            if (m_scene->renderTargets.count(el.second) == 0) continue;
            const auto& rt = m_scene->renderTargets[el.second];

            const auto name_resolution = WE_GLTEX_RESOLUTION_NAMES[el.first];
            if (existsOp(name_resolution)) {
                std::array<uint16_t, 4> resolution_uint(
                    { rt.width, rt.height, rt.width, rt.height });
                updateOp(name_resolution, ShaderValue(array_cast<float>(resolution_uint)));
            }
            const auto name_mipmap = WE_GLTEX_MIPMAPINFO_NAMES[el.first];
            if (existsOp(name_mipmap)) {
                updateOp(name_mipmap, (float)rt.mipmap_level);
            }
        }
        if (nodeData.puppet && existsOp(G_BONES)) {
            auto data = nodeData.puppet->genFrame(nodeData.puppet_layers, m_scene->frameTime);
            updateOp(G_BONES, Span<float> { data[0].data(), data.size() * 16 });
        }
    }

    bool reqMI   = existsOp(G_MI);
    bool reqM    = existsOp(G_M);
    bool reqAM   = existsOp(G_AM);
    bool reqMVP  = existsOp(G_MVP);
    bool reqMVPI = existsOp(G_MVPI);

    Matrix4d viewProTrans = camera->GetViewProjectionMatrix();

    if (existsOp(G_VP)) {
        updateOp(G_VP, ShaderValue::fromMatrix(viewProTrans));
    }
    if (reqM || reqMVP || reqMI || reqMVPI) {
        Matrix4d modelTrans = pNode->GetLocalTrans().cast<double>();
        if (hasNodeData && cam_name != "effect") {
            const auto& nodeData = m_nodeDataMap.at(pNode);
            if (m_parallax.enable) {
                Vector3f nodePos = pNode->Translate();
                Vector2f depth(&nodeData.parallaxDepth[0]);
                Vector2f ortho { (float)m_scene->ortho[0], (float)m_scene->ortho[1] };
                Vector2f mouseVec =
                    (Vector2f { 0.5f, 0.5f } - Vector2f(&m_mousePos[0])).cwiseProduct(ortho);
                mouseVec *= m_parallax.mouseinfluence;
                Vector3f camPos = camera->GetPosition().cast<float>();
                Vector2f paraVec =
                    (nodePos.head<2>() - camPos.head<2>() + mouseVec).cwiseProduct(depth) *
                    m_parallax.amount;
                modelTrans =
                    Affine3d(Translation3d(Vector3d(paraVec.x(), paraVec.y(), 0.0f))).matrix() *
                    modelTrans;
            }
        }

        if (reqM) updateOp(G_M, ShaderValue::fromMatrix(modelTrans));
        if (reqAM) updateOp(G_AM, ShaderValue::fromMatrix(modelTrans));
        if (reqMI) updateOp(G_MI, ShaderValue::fromMatrix(modelTrans.inverse()));
        if (reqMVP) {
            Matrix4d mvpTrans = viewProTrans * modelTrans;
            updateOp(G_MVP, ShaderValue::fromMatrix(mvpTrans));
            if (reqMVPI) updateOp(G_MVPI, ShaderValue::fromMatrix(mvpTrans.inverse()));
        }
    }

    //	g_EffectTextureProjectionMatrix
    // shadervs.push_back({"g_EffectTextureProjectionMatrixInverse",
    // ShaderValue::ValueOf(Eigen::Matrix4f::Identity())});
    if (existsOp(G_TIME)) updateOp(G_TIME, (float)m_scene->elapsingTime);

    if (existsOp(G_DAYTIME)) updateOp(G_DAYTIME, (float)m_dayTime);

    if (existsOp(G_POINTERPOSITION)) updateOp(G_POINTERPOSITION, m_mousePos);

    if (existsOp(G_TEXELSIZE)) updateOp(G_TEXELSIZE, m_texelSize);

    if (existsOp(G_TEXELSIZEHALF))
        updateOp(G_TEXELSIZEHALF, std::array { m_texelSize[0] / 2.0f, m_texelSize[1] / 2.0f });

    if (existsOp(G_SCREEN))
        updateOp(G_SCREEN,
                 std::array<float, 3> {
                     m_screen_size[0], m_screen_size[1], m_screen_size[0] / m_screen_size[1] });

    for (auto& [i, sp] : sprites) {
        const auto& f      = sp.GetAnimateFrame(m_scene->frameTime);
        auto        grot   = WE_GLTEX_ROTATION_NAMES[i];
        auto        gtrans = WE_GLTEX_TRANSLATION_NAMES[i];
        updateOp(grot, std::array { f.xAxis[0], f.xAxis[1], f.yAxis[0], f.yAxis[1] });
        updateOp(gtrans, std::array { f.x, f.y });
    }

    if (existsOp(G_LP)) {
        std::array<float, 16> lights { 0 };
        std::array<float, 12> lights_color { 0 };
        uint                  i = 0;
        for (auto& l : m_scene->lights) {
            assert(l->node() != nullptr);
            if (i == 3) break;
            const auto& trans = l->node()->Translate();
            const auto& color = l->premultipliedColor();
            std::copy(trans.begin(), trans.end(), lights.begin() + i * 4);
            std::copy(color.begin(), color.end(), lights_color.begin() + i * 4);
            i++;
        }
        updateOp(G_LP, lights);
        updateOp(G_LCP, lights_color);
    }
}

void WPShaderValueUpdater::SetNodeData(void* nodeAddr, const WPShaderValueData& data) {
    m_nodeDataMap[nodeAddr] = data;
}

void WPShaderValueUpdater::SetTexelSize(float x, float y) { m_texelSize = { x, y }; }