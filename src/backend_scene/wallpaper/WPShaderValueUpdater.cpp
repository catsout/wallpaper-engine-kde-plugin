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

void WPShaderValueUpdater::InitUniforms(SceneNode* pNode, const ExistsUniformOp& existsOp) {
    m_nodeUniformInfoMap[pNode] = WPUniformInfo();
    auto& info                  = m_nodeUniformInfoMap[pNode];
    info.has_MI                 = existsOp(G_MI);
    info.has_M                  = existsOp(G_M);
    info.has_AM                 = existsOp(G_AM);
    info.has_MVP                = existsOp(G_MVP);
    info.has_MVPI               = existsOp(G_MVPI);
    info.has_VP                 = existsOp(G_VP);

    info.has_BONES           = existsOp(G_BONES);
    info.has_TIME            = existsOp(G_TIME);
    info.has_DAYTIME         = existsOp(G_DAYTIME);
    info.has_POINTERPOSITION = existsOp(G_POINTERPOSITION);
    info.has_TEXELSIZE       = existsOp(G_TEXELSIZE);
    info.has_TEXELSIZEHALF   = existsOp(G_TEXELSIZEHALF);
    info.has_SCREEN          = existsOp(G_SCREEN);
    info.has_LP              = existsOp(G_LP);

    std::accumulate(begin(info.texs), end(info.texs), 0, [&existsOp](uint index, auto& value) {
        value.has_resolution = existsOp(WE_GLTEX_RESOLUTION_NAMES[index]);
        value.has_mipmap     = existsOp(WE_GLTEX_MIPMAPINFO_NAMES[index]);
        return index + 1;
    });
}

void WPShaderValueUpdater::UpdateUniforms(SceneNode* pNode, sprite_map_t& sprites,
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

    assert(exists(m_nodeUniformInfoMap, pNode));
    const auto& info = m_nodeUniformInfoMap[pNode];

    bool hasNodeData = exists(m_nodeDataMap, pNode);
    if (hasNodeData) {
        auto& nodeData = m_nodeDataMap.at(pNode);
        for (const auto& el : nodeData.renderTargets) {
            if (m_scene->renderTargets.count(el.second) == 0) continue;
            const auto& rt = m_scene->renderTargets[el.second];

            const auto& unifrom_tex = info.texs[el.first];

            if (unifrom_tex.has_resolution) {
                std::array<uint16_t, 4> resolution_uint(
                    { rt.width, rt.height, rt.width, rt.height });
                updateOp(WE_GLTEX_RESOLUTION_NAMES[el.first],
                         ShaderValue(array_cast<float>(resolution_uint)));
            }
            if (unifrom_tex.has_mipmap) {
                updateOp(WE_GLTEX_MIPMAPINFO_NAMES[el.first], (float)rt.mipmap_level);
            }
        }
        if (nodeData.puppet && info.has_BONES) {
            auto data = nodeData.puppet->genFrame(nodeData.puppet_layers, m_scene->frameTime);
            updateOp(G_BONES, Span<const float> { data[0].data(), data.size() * 16 });
        }
    }

    bool reqMI   = info.has_MI;
    bool reqM    = info.has_M;
    bool reqAM   = info.has_AM;
    bool reqMVP  = info.has_MVP;
    bool reqMVPI = info.has_MVPI;

    Matrix4d viewProTrans = camera->GetViewProjectionMatrix();

    if (info.has_VP) {
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
                // flip mouse y axis
                Vector2f mouseVec =
                    Scaling(1.0f, -1.0f) * (Vector2f { 0.5f, 0.5f } - Vector2f(&m_mousePos[0]));
                mouseVec        = mouseVec.cwiseProduct(ortho) * m_parallax.mouseinfluence;
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
    if (info.has_TIME) updateOp(G_TIME, (float)m_scene->elapsingTime);

    if (info.has_DAYTIME) updateOp(G_DAYTIME, (float)m_dayTime);

    if (info.has_POINTERPOSITION) updateOp(G_POINTERPOSITION, m_mousePos);

    if (info.has_TEXELSIZE) updateOp(G_TEXELSIZE, m_texelSize);

    if (info.has_TEXELSIZEHALF)
        updateOp(G_TEXELSIZEHALF, std::array { m_texelSize[0] / 2.0f, m_texelSize[1] / 2.0f });

    if (info.has_SCREEN)
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

    if (info.has_LP) {
        std::array<float, 16> lights { 0 };
        std::array<float, 12> lights_color { 0 };
        uint                  i = 0;
        for (auto& l : m_scene->lights) {
            if (i == 4) break;
            assert(l->node() != nullptr);
            const auto& trans = l->node()->Translate();
            std::copy(trans.begin(), trans.end(), lights.begin() + i * 4);
            if (i < 3) {
                const auto& color = l->premultipliedColor();
                std::copy(color.begin(), color.end(), lights_color.begin() + i * 4);
            }
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
