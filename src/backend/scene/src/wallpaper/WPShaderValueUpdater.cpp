#include "WPShaderValueUpdater.h"
#include "Scene.h"
#include "SpriteAnimation.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>
#include <ctime>

#define G_M "g_ModelMatrix"
#define G_VP "g_ViewProjectionMatrix"
#define G_MVP "g_ModelViewProjectionMatrix"

#define G_MI "g_ModelMatrixInverse"
#define G_MVPI "g_ModelViewProjectionMatrixInverse"

#define CONTAINS(s, v) (s.count(v) == 1)

using namespace wallpaper;

void WPShaderValueUpdater::FrameBegin() {
	using namespace std::chrono;
	auto nowTime = system_clock::to_time_t(system_clock::now());
	auto cTime = std::localtime(&nowTime);
	m_dayTime = (((cTime->tm_hour * 60) + cTime->tm_min) * 60 + cTime->tm_sec) / (24.0f*60.0f*60.0f);
}

void WPShaderValueUpdater::FrameEnd() {
}


void WPShaderValueUpdater::MouseInput(double x, double y) {
	m_mousePos[0] = x;
	m_mousePos[1] = y;
}

void WPShaderValueUpdater::UpdateShaderValues(SceneNode* pNode, SceneShader* pShader) {
	if(!pNode->Mesh()) return;

	const SceneCamera* camera;
	if(!pNode->Camera().empty()) {
		camera = m_scene->cameras.at(pNode->Camera()).get();
	}
	else
		camera = m_scene->activeCamera;

	if(!camera) return;

	const std::string gtex = "g_Texture";

	
	auto* material = pNode->Mesh()->Material();
	if(!material) return;
	auto& shadervs = material->customShader.updateValueList;	
	const auto& valueSet = material->customShader.valueSet;
	bool hasNodeData = CONTAINS(m_nodeDataMap, pNode);
	if(hasNodeData) {
		const auto& nodeData = m_nodeDataMap.at(pNode);
		for(const auto& el:nodeData.renderTargetResolution) {
			if(m_scene->renderTargets.count(el.second) == 0) continue;
			std::string name = gtex + std::to_string(el.first) + "Resolution";
			if(!CONTAINS(valueSet, name)) continue;

			const auto& rt = m_scene->renderTargets.at(el.second);
			std::vector<uint32_t> resolution_uint({
				rt.width, rt.height, 
				rt.width, rt.height
			});
			std::vector<float> resolution(resolution_uint.begin(), resolution_uint.end());
			shadervs.push_back({name, resolution});
		}
	}

	bool reqMI = CONTAINS(valueSet, G_MI);
	bool reqM = CONTAINS(valueSet, G_M);
	bool reqMVP = CONTAINS(valueSet, G_MVP);
	bool reqMVPI CONTAINS(valueSet, G_MVPI);
	const auto& viewProTrans = camera->GetViewProjectionMatrix();

	if(CONTAINS(valueSet, G_VP)) {
		shadervs.push_back({G_VP, ShaderValue::ValueOf(viewProTrans)});
	}
	if(reqM || reqMVP || reqMI || reqMVPI) {
		glm::mat4 modelTrans = pNode->GetLocalTrans();
		if(hasNodeData) {
			const auto& nodeData = m_nodeDataMap.at(pNode);
			if(m_parallax.enable) {
				glm::vec3 nodePos = glm::make_vec3(&(pNode->Translate())[0]);
				glm::vec2 depth = glm::make_vec2(&nodeData.parallaxDepth[0]);
				glm::vec2 ortho = glm::vec2{m_ortho[0], m_ortho[1]};
				glm::vec2 mouseVec = (glm::vec2{0.5f, 0.5f} - glm::make_vec2(&m_mousePos[0])) * ortho;
				mouseVec *= m_parallax.mouseinfluence;
				const auto& camPos = camera->GetPosition();
				glm::vec2 paraVec = (nodePos.xy() - camPos.xy() + mouseVec) * depth * m_parallax.amount;
				modelTrans = glm::translate(glm::mat4(1.0f), glm::vec3(paraVec, 0.0f)) * modelTrans;
			}
		}
		if(reqM) shadervs.push_back({G_M, ShaderValue::ValueOf(modelTrans)});
		if(reqMI) shadervs.push_back({G_MI, ShaderValue::ValueOf(glm::inverse(modelTrans))});
		if(reqMVP) {
			auto mvpTrans = viewProTrans * modelTrans;
			shadervs.push_back({G_MVP, ShaderValue::ValueOf(mvpTrans)});
			if(reqMVPI) shadervs.push_back({G_MVPI, ShaderValue::ValueOf(glm::inverse(mvpTrans))});
		}
	}
	//	g_EffectTextureProjectionMatrix
	if(CONTAINS(valueSet, "g_Time"))
		shadervs.push_back({"g_Time", {(float)m_scene->elapsingTime}});

	if(CONTAINS(valueSet, "g_DayTime"))
		shadervs.push_back({"g_DayTime", {(float)m_dayTime}});

	if(CONTAINS(valueSet, "g_PointerPosition"))
		shadervs.push_back({"g_PointerPosition", m_mousePos});

	if(CONTAINS(valueSet, "g_TexelSize"))
		shadervs.push_back({"g_TexelSize", m_texelSize});

	if(CONTAINS(valueSet, "g_TexelSizeHalf"))
		shadervs.push_back({"g_TexelSizeHalf", {m_texelSize[0]/2.0f, m_texelSize[1]/2.0f}});

	if(material->hasSprite) {
		for(int32_t i=0;i<material->textures.size();i++) {
			const auto& texname = material->textures.at(i);
			if(m_scene->textures.count(texname) != 0) {
				auto& ptex = m_scene->textures.at(texname);
				if(ptex->isSprite) {
					auto& sp = ptex->spriteAnim;
					const auto& frame = sp.GetAnimateFrame(m_scene->frameTime);
					auto grot = gtex + std::to_string(i) + "Rotation";
					auto gtrans = gtex + std::to_string(i) + "Translation";
					shadervs.push_back({grot, {frame.width, 0, 0, frame.height}});
					shadervs.push_back({gtrans, {frame.x, frame.y}});
				}
			}
		}
	}
}

void WPShaderValueUpdater::SetNodeData(void* nodeAddr, const WPShaderValueData& data) {
	m_nodeDataMap[nodeAddr] = data;
}

void WPShaderValueUpdater::SetTexelSize(float x, float y) {
	m_texelSize = {x, y};
}
