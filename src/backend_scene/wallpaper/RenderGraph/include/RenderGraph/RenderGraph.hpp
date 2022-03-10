#pragma once

#include "Pass.hpp"
#include "TexNode.hpp"
#include "PassNode.hpp"

#include "Utils/MapSet.hpp"
#include "Utils/span.hpp"

namespace wallpaper
{
namespace rg
{

class RenderGraph;
class RenderGraphBuilder {
public:
    RenderGraphBuilder(RenderGraph&);

    TexNode* createTexNode(const TexNode::Desc&, bool write=false);
    void read(TexNode*);
    void write(TexNode*);
    const PassNode& workPassNode() const;
    void setWorkPassNode(PassNode*);
private:
    TexNode* createNewTexNode(const TexNode::Desc&);

    RenderGraph& m_rg;
    PassNode* m_passnode_wip {nullptr};
};

class RenderGraph {
public:
    RenderGraph();

    PassNode* getPassNode(NodeID) const;
    TexNode* getTexNode(NodeID) const;
    Pass* getPass(NodeID) const;

    // all render pass
    std::vector<NodeID> topologicalOrder() const;
    std::vector<std::vector<TexNode*>> getLastReadTexs(Span<NodeID>) const; 

  	void ToGraphviz(std::string_view path) const {
		m_dg.ToGraphviz(path);	
	};

    template <typename TPass, typename CB>
    PassNode* addPass(std::string_view name, PassNode::Type type, CB&& callback) {
        using Desc = typename TPass::Desc;
        
        auto* node = PassNode::addPassNode(m_dg, type);
        node->setName(name);
        markPassNode(node->ID());
        RenderGraphBuilder builder(*this);
        builder.setWorkPassNode(node);
        {
            Desc desc {};
            callback(builder, desc);
            m_map_pass[node->ID()] = std::make_shared<TPass>(desc);
        }
        return node;
    }
private:
    friend class RenderGraphBuilder;
    void markPassNode(NodeID);
    bool isPassNode(NodeID) const;

    DependencyGraph m_dg;
    Set<NodeID> m_set_passnode;

    Map<std::string, NodeID> m_key_texnode;

    Map<NodeID, std::shared_ptr<Pass>> m_map_pass;
};

}
}