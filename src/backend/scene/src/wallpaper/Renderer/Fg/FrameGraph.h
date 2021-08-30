#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include "ResourceOper.h"
#include "PassNode.h"
#include "ResourceNode.h"
#include "DependencyGraph.h"

#include "Resource.h"
#include "TextureResource.h"
#include "TextureCache.h"
#include "RenderTargetResource.h"

#include "Interface/IGraphicManager.h"

namespace wallpaper {
namespace fg {

class FrameGraph;

class FrameGraphBuilder {
public:
	FrameGraphBuilder(FrameGraph& fg, PassNode* node):m_fg(fg),m_pPassNode(node) {};
	FrameGraphResource Read(FrameGraphResource);
	FrameGraphMutableResource Read(FrameGraphMutableResource);
	FrameGraphMutableResource Write(FrameGraphMutableResource);

	
	FrameGraphMutableResource CreateTexture(TextureResource::Desc);
	FrameGraphMutableResource CreateTexture(FrameGraphResource);
	std::shared_ptr<RenderPassData> UseRenderPass(RenderPassData);
private:
	NodeID InitResource(ResourceOper*);
	FrameGraph& m_fg;
	PassNode* m_pPassNode;
};

class FrameGraphResourceManager {
public:
	FrameGraphResourceManager(FrameGraph& fg, PassNode* node):m_fg(fg),m_pPassNode(node) {};
	~FrameGraphResourceManager() = default;

	const TextureResource* GetTexture(FrameGraphResource);
private:
	FrameGraph& m_fg;
	PassNode* m_pPassNode;
};

class FrameGraph {
	friend class FrameGraphBuilder;
	friend class FrameGraphResourceManager;
public:
	FrameGraph() = default;
	~FrameGraph() = default;
	FrameGraph(const FrameGraph&) = delete;
	FrameGraph& operator=(FrameGraph&) = delete;

	template<typename Data, typename Setup, typename Execute>
	Pass<Data, Execute>& AddPass(std::string_view name, Setup setup, Execute&& execute);

	FrameGraphMutableResource AddMovePass(FrameGraphMutableResource);

	void Compile();
	void Execute(IGraphicManager&);

	const DependencyGraph& Graph() const { return m_graph; }

	void ToGraphviz() const {
		m_graph.ToGraphviz("");	
	};
private:
	struct ResourceData {
	};
	inline PassNode* AddRenderPassNode(std::string_view name, PassBase* pass);

	std::vector<NodeID> m_sorted;
	std::vector<std::vector<ResourceNode::ResourceHandle>> m_releaseAfter;

	std::vector<PassBase*> m_passes;

	DependencyGraph m_graph;
	std::vector<NodeID> m_passNodes;
	std::vector<NodeID> m_resNodes;

	std::unordered_set<NodeID> m_resNodeSet;

	ResourceContainer<TextureResource> m_textures;
	TextureCache m_texCache;
};

inline PassNode* FrameGraph::AddRenderPassNode(std::string_view name, PassBase* pass) {
	auto upNode = std::make_unique<PassNode>(name, pass);
	auto* pNode = upNode.get();

	m_passNodes.push_back(m_graph.AddNode(std::move(upNode)));
	return pNode;
}


template<typename TData, typename TSetup, typename TExecute>
Pass<TData, TExecute>& FrameGraph::AddPass(std::string_view name, TSetup setup, TExecute&& execute) {
	auto* const pass = new Pass<TData, TExecute>(std::forward<TExecute>(execute));
	m_passes.push_back(pass);
	FrameGraphBuilder builder(*this, AddRenderPassNode(name, pass));
	setup(builder, const_cast<TData&>(pass->GetData()));
	return *pass;
}
}
}