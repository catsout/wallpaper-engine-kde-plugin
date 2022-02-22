#pragma once
#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <memory>
#include "Pass.h"
#include "DependencyGraph.h"
#include "ResourceOper.h"

namespace wallpaper {
namespace fg {

class ResourceNode;

struct RenderPassData {
	struct ViewPort {
		uint16_t x {0};
		uint16_t y {0};
		uint16_t width {1};
		uint16_t height {1};
	};
	HwRenderTargetHandle target;
	std::array<FrameGraphResource, MaxAttachmentNum> attachments; 
	IGraphicManager::RenderTargetDesc lastGMDesc;
	ViewPort viewPort;
};

class PassNode : public DependencyGraph::Node {
public:
	PassNode(std::string_view name, PassBase* pass):m_name(name),m_pPass(pass) {}
	PassBase* Pass() const { return m_pPass; }

	void AddInput(ResourceNode* r) { m_inputs.push_back(r); }
	auto Inputs() const { return m_inputs; }

	auto GetRenderPassData() const { return m_rendpassData; }
	void SetRenderPassData(std::shared_ptr<RenderPassData> data) { m_rendpassData = data; }

	std::string ToGraphviz() const override {
		return GraphID() + "[label=\""+m_name+"\" shape=box]";
	}
private:
	std::string m_name;
	PassBase* m_pPass;

	std::shared_ptr<RenderPassData> m_rendpassData;

	std::vector<ResourceNode*> m_inputs;
};
}
}