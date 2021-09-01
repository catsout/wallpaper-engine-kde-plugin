#include "FrameGraph.h"
#include <algorithm>
#include "Log.h"

using namespace wallpaper::fg;

ResourceNode* GetResourceNode(DependencyGraph& dg, NodeID id) {
	return static_cast<ResourceNode*>(dg.GetNode(id));
}

NodeID FrameGraphBuilder::InitResource(ResourceOper* rsc) {
	rsc->Initialize(m_fg.m_resNodes.size());	
	m_fg.m_resNodes.push_back(m_fg.m_graph.AddNode(std::make_unique<ResourceNode>()));
	return m_fg.m_resNodes.at(rsc->Index());
}

FrameGraphResource FrameGraphBuilder::Read(FrameGraphResource rsc) {
	if(!rsc.IsInitialed()) InitResource(&rsc);

	NodeID recN = m_fg.m_resNodes[rsc.Index()];
	m_pPassNode->AddInput(GetResourceNode(m_fg.m_graph, recN));
	m_fg.m_graph.Connect(recN, m_pPassNode->ID());
	return rsc;
}

FrameGraphMutableResource FrameGraphBuilder::Read(FrameGraphMutableResource rsc) {
	if(!rsc.IsInitialed()) InitResource(&rsc);

	NodeID recN = m_fg.m_resNodes[rsc.Index()];
	m_fg.m_graph.Connect(recN, m_pPassNode->ID());


	ResourceNode* recNode = GetResourceNode(m_fg.m_graph, recN);
	m_pPassNode->AddInput(recNode);
	if(recNode->IsFromMoved()) {
		m_fg.m_graph.Connect(m_pPassNode->ID(), recNode->GetMover()->ID());
	}
	return rsc;
}

FrameGraphMutableResource FrameGraphBuilder::Write(FrameGraphMutableResource rsc) {
	if(!rsc.IsInitialed()) InitResource(&rsc);

	NodeID recN = m_fg.m_resNodes[rsc.Index()];
	ResourceNode* recNode = GetResourceNode(m_fg.m_graph, recN);

	if(recNode->IsWrited()) {
		assert(false);
		// log error
		return {};	
	}
	if(recNode->IsToMoved()) {
		m_fg.m_graph.Connect(recNode->GetMover()->ID(), m_pPassNode->ID());
	}

	m_fg.m_graph.Connect(m_pPassNode->ID(), recN);
	return rsc;
}



FrameGraphMutableResource FrameGraph::AddMovePass(FrameGraphMutableResource rscFrom) {
	if(!rscFrom.IsInitialed()) {
		return {};
	}
	ResourceNode* resNodeFrom = GetResourceNode(m_graph, m_resNodes[rscFrom.Index()]);
	if(resNodeFrom->IsFromMoved()) return {};

	auto* const pass = new MovePass();
	m_passes.push_back(pass);
	auto* const moveNode = AddRenderPassNode("move", pass);

	resNodeFrom->SetMoved(moveNode, true);

	// make sure all readpass before move
	for(NodeID out:m_graph.GetNodeOut(resNodeFrom->ID())) {
		if(std::count(m_passNodes.begin(), m_passNodes.end(), out) > 0) {
			m_graph.Connect(out, moveNode->ID());
		}
	}
	m_graph.Connect(resNodeFrom->ID(), moveNode->ID());

	FrameGraphMutableResource toRes;
	toRes.Initialize(m_resNodes.size());
	m_resNodes.push_back(m_graph.AddNode(std::make_unique<ResourceNode>()));
	m_graph.Connect(moveNode->ID(), m_resNodes.back());

	ResourceNode* resNodeTo = GetResourceNode(m_graph, m_resNodes.back());
	resNodeTo->SetMoved(moveNode, false);
	resNodeTo->LinkResource(resNodeFrom->GetResourceHandle());
	m_textures.AddLinkedNode({resNodeTo->GetResourceHandle().idx}, resNodeTo->ID());
	resNodeTo->SetName(resNodeFrom->m_name);
	return toRes;
}

FrameGraphMutableResource FrameGraphBuilder::CreateTexture(TextureResource::Desc desc) {
	FrameGraphMutableResource rsc;
	auto nodeId = InitResource(&rsc);
	auto tex = TextureResource();
	tex.desc = desc;
	//tex.initOp = init;
	ResourceNode* resNode = GetResourceNode(m_fg.m_graph, nodeId);
	resNode->LinkResource(m_fg.m_textures.New(tex, nodeId));
	resNode->SetName(tex.desc.name);
	return rsc;
}


FrameGraphMutableResource FrameGraphBuilder::CreateTexture(FrameGraphResource src) {
	FrameGraphMutableResource dst;
	auto nodeId = InitResource(&dst);
	auto tex = TextureResource();
	{
		ResourceNode* srcNode = GetResourceNode(m_fg.m_graph, m_fg.m_resNodes[src.Index()]);
		auto* srcTex = m_fg.m_textures.Get({srcNode->GetResourceHandle().idx});
		tex.desc = srcTex->desc;
	} 
	{
		tex.desc.name += "_copy";
		tex.desc.temperary = true;
		ResourceNode* dstNode = GetResourceNode(m_fg.m_graph, nodeId);
		dstNode->LinkResource(m_fg.m_textures.New(tex, nodeId));
		dstNode->SetName(tex.desc.name);
	}
	return dst;
}


std::shared_ptr<RenderPassData> FrameGraphBuilder::UseRenderPass(RenderPassData data) {
	m_pPassNode->SetRenderPassData(std::make_shared<RenderPassData>(data));
	return m_pPassNode->GetRenderPassData();
}

void FrameGraph::Compile() {
	auto sorted = m_graph.TopologicalOrder();
	m_resNodeSet = std::unordered_set<NodeID>(m_resNodes.begin(), m_resNodes.end());
	{
		std::vector<NodeID> result;
		std::unordered_set<NodeID> pidSet(m_passNodes.begin(), m_passNodes.end());
		result.reserve(sorted.size());

		// sort render pass node
		std::copy_if(sorted.begin(), sorted.end(), std::back_inserter(result), [&](NodeID id) {
			return pidSet.count(id) > 0;
		});
		m_sorted = result;
	}
	{
		int pos = 0;
		std::unordered_map<uint16_t,int> orderMap;
		std::for_each(m_sorted.begin(), m_sorted.end(), [&](NodeID n){
			PassNode* node = static_cast<PassNode*>(m_graph.GetNode(n));
			for(auto& in:node->Inputs()) {
				if(m_textures.Check({in->GetResourceHandle().idx}, in->ID())) {
					auto *tex = m_textures.Get( {in->GetResourceHandle().idx} );
					if(tex && tex->desc.temperary) {
						uint16_t handle = in->GetResourceHandle().idx;
						orderMap[handle] = pos;
					}
				}
			}
			pos++;
		});
		m_releaseAfter.clear();
		m_releaseAfter.resize(m_sorted.size());
		for(const auto& el:orderMap) {
			m_releaseAfter[el.second].push_back({ el.first });
		}
	}
}

void FrameGraph::Execute(IGraphicManager& gm) {
	int pos=-1;
	for(auto& n:m_sorted) {
		pos++;

		PassNode* node = static_cast<PassNode*>(m_graph.GetNode(n));
		{
			auto loadTex = [this, &gm](ResourceNode* node) {
				if(m_textures.Check({node->GetResourceHandle().idx}, node->ID())) {
					auto* tex = m_textures.Get({node->GetResourceHandle().idx});
					if(!tex->Initialed()) {
						tex->Initialize();
						if(tex->desc.UpdateDescOp) {
							tex->desc.UpdateDescOp(tex->desc);
						}
						if(tex->desc.getImgOp) {
							auto img = tex->desc.getImgOp();
							tex->handle = m_texCache.Query(*img, gm);
							tex->desc.width = img->header.width;
							tex->desc.height = img->header.height;
						} else {
							tex->handle = m_texCache.Query(tex->desc, gm);
						}
					}
				}
			};
			for(auto& in:node->Inputs()) {
				loadTex(in);
			}
			auto outs = m_graph.GetNodeOut(node->ID());
			for(auto& out:outs) {
				if(m_resNodeSet.count(out) > 0) {
					auto* resNode = GetResourceNode(m_graph, out);
					//LOG_ERROR(std::to_string(resNode->GetResourceHandle().idx) + ":" + std::to_string(resNode->ID()));
					//assert(false);
					loadTex(resNode);
				}
			}
		}
		FrameGraphResourceManager rm(*this, node);
		{
			auto* renderdata = node->GetRenderPassData().get();
			if(renderdata != nullptr) {
				if(!HwRenderTargetHandle::IsInvalied(renderdata->target)) {
					gm.DestroyRenderTarget(renderdata->target);
					renderdata->target = HwRenderTargetHandle();
				} 
				IGraphicManager::RenderTargetDesc desc; 
				for(int i=0;i<renderdata->attachments.size();i++) {
					if(!renderdata->attachments[i].IsInitialed()) break;
					auto* tex = rm.GetTexture(renderdata->attachments[i]);
					if(tex != nullptr) {
						if(!tex->Initialed()) assert(false);
						desc.attachs[i] = tex->handle;

						if(i == 0) {
							desc.width = tex->desc.width;
							desc.height = tex->desc.height;
						}
					}
				}
				renderdata->target = gm.CreateRenderTarget(desc);
			}
		}
		node->Pass()->execute(rm);
		{
			for(const auto& el:m_releaseAfter[pos]) {
				auto* tex = m_textures.Get({el.idx});
				m_texCache.Release(tex->handle);
				tex->Release();
			}
		}
	}
	//m_texCache.ClearUnused(gm);
}
const TextureResource* FrameGraphResourceManager::GetTexture(FrameGraphResource rsc) {
	assert(rsc.IsInitialed());
	auto* rscNode = GetResourceNode(m_fg.m_graph, m_fg.m_resNodes.at(rsc.Index()));
	if(m_fg.m_textures.Check({ rscNode->GetResourceHandle().idx }, rscNode->ID())) {
		return m_fg.m_textures.Get({ rscNode->GetResourceHandle().idx });
	}
	return nullptr;
}