#include "WPShaderValueUpdater.h"
#include "Scene.h"
#include "SpriteAnimation.h"
#include "common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


using namespace wallpaper;

void WPShaderValueUpdater::FrameBegin() {
	m_timeDiff = m_scene->elapsingTime - m_lastTime;
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
			for(const auto& el:nodeData.resolutions) {
				shadervs.push_back({gtex + std::to_string(el.index) + "Resolution", {el.width, el.height, el.mapWidth, el.mapHeight}});
			}
			if(m_parallax.enable) {
				const auto& translate = pNode->Translate();
				glm::vec3 nodePos = glm::make_vec3(&translate[0]);
				glm::vec2 depth = glm::make_vec2(&nodeData.parallaxDepth[0]);
				glm::vec2 mouseVec = glm::vec2{camera->Width()/2.0f, camera->Height()/2.0f} - glm::make_vec2(&m_mousePos[0]);
				mouseVec *= m_parallax.mouseinfluence;
				if(camera) {
					const auto& camPos = camera->GetPosition();
					glm::vec2 paraVec = (nodePos.xy() - camPos.xy() + mouseVec) * depth * m_parallax.amount;
					modelTrans = glm::translate(glm::mat4(1.0f), glm::vec3(paraVec, 0.0f)) * modelTrans;
				}
			}
		}
		auto viewtrans = glm::mat4(1.0f);
		viewtrans = camera->GetViewMatrix();
		modelTrans = camera->GetProjectionMatrix() * viewtrans * modelTrans;
		shadervs.push_back({"g_ModelViewProjectionMatrix", ShaderValue::ValueOf(modelTrans)});

		shadervs.push_back({"g_Time", {(float)m_scene->elapsingTime}});

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
