#include "WPShaderValueUpdater.h"
#include "Scene.h"
#include "SpriteAnimation.h"
#include "common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>
#include <ctime>


using namespace wallpaper;

void WPShaderValueUpdater::FrameBegin() {
	using namespace std::chrono;
	m_timeDiff = m_scene->elapsingTime - m_lastTime;
	auto nowTime = system_clock::to_time_t(system_clock::now());
	auto cTime = std::localtime(&nowTime);
	m_dayTime = (((cTime->tm_hour * 60) + cTime->tm_min) * 60 + cTime->tm_sec) / (24.0f*60.0f*60.0f);
}

void WPShaderValueUpdater::FrameEnd() {
	m_lastTime = m_scene->elapsingTime;
}


void WPShaderValueUpdater::MouseInput(double x, double y) {
	m_mousePos[0] = x;
	m_mousePos[1] = y;
}

void WPShaderValueUpdater::UpdateShaderValues(SceneNode* pNode, SceneShader* pShader) {
	if(!pNode->Mesh()) return;
	glm::mat4 modelTrans = pNode->GetLocalTrans();
	const SceneCamera* camera;
	if(!pNode->Camera().empty()) {
		camera = m_scene->cameras.at(pNode->Camera()).get();
	}
	else
		camera = m_scene->activeCamera;

	if(!camera) return;

	const std::string gtex = "g_Texture";

	
	auto* material = pNode->Mesh()->Material();
	if(material) {
		auto& shadervs = material->customShader.updateValueList;	
		if(m_nodeDataMap.count(pNode) != 0) {
			const auto& nodeData = m_nodeDataMap.at(pNode);
			for(const auto& el:nodeData.renderTargetResolution) {
				if(m_scene->renderTargets.count(el.second) == 0) continue;
				const auto& rt = m_scene->renderTargets.at(el.second);
				std::vector<float> resolution({
					(float)rt.width, 
					(float)rt.height, 
					(float)rt.width, 
					(float)rt.height
				});
				shadervs.push_back({gtex + std::to_string(el.first) + "Resolution", resolution});
			}
			if(m_parallax.enable) {
				const auto& translate = pNode->Translate();
				glm::vec3 nodePos = glm::make_vec3(&translate[0]);
				glm::vec2 depth = glm::make_vec2(&nodeData.parallaxDepth[0]);
				glm::vec2 camOrtho = glm::vec2{camera->Width(), camera->Height()};
				glm::vec2 mouseVec = (glm::vec2{0.5f, 0.5f} - glm::make_vec2(&m_mousePos[0])) * camOrtho;
				mouseVec *= m_parallax.mouseinfluence;
				if(camera) {
					const auto& camPos = camera->GetPosition();
					glm::vec2 paraVec = (nodePos.xy() - camPos.xy() + mouseVec) * depth * m_parallax.amount;
					modelTrans = glm::translate(glm::mat4(1.0f), glm::vec3(paraVec, 0.0f)) * modelTrans;
				}
			}
		}
		const auto& viewTrans = camera->GetViewMatrix();
		auto mvpTrans = camera->GetProjectionMatrix() * viewTrans * modelTrans;

		shadervs.push_back({"g_ModelMatrix", ShaderValue::ValueOf(modelTrans)});
		shadervs.push_back({"g_ModelMatrixInverse", ShaderValue::ValueOf(glm::inverse(modelTrans))});
		shadervs.push_back({"g_ViewProjectionMatrix", ShaderValue::ValueOf(viewTrans)});
		shadervs.push_back({"g_ModelViewProjectionMatrix", ShaderValue::ValueOf(mvpTrans)});
		shadervs.push_back({"g_ModelViewProjectionMatrixInverse", ShaderValue::ValueOf(glm::inverse(mvpTrans))});
				//	g_EffectTextureProjectionMatrix

		shadervs.push_back({"g_Time", {(float)m_scene->elapsingTime}});
		shadervs.push_back({"g_DayTime", {(float)m_dayTime}});
		shadervs.push_back({"g_PointerPosition", m_mousePos});

		for(int32_t i=0;i<material->textures.size();i++) {
			const auto& texname = material->textures.at(i);
			if(m_scene->textures.count(texname) != 0) {
				auto& ptex = m_scene->textures.at(texname);
				if(ptex->isSprite) {
					auto& sp = ptex->spriteAnim;
					const auto& frame = sp.GetAnimateFrame(m_timeDiff);
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
