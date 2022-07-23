#pragma once

#include "PassNode.hpp"

#include "TexNode.hpp"
#include "Utils/MapSet.hpp"
#include "Utils/span.hpp"

namespace wallpaper
{
namespace rg
{

class Pass;
class RenderGraph;
class RenderGraphBuilder {
public:
    RenderGraphBuilder(RenderGraph&);

    TexNode*        createTexNode(const TexNode::Desc&, bool write = false);
    void            read(TexNode*);
    void            write(TexNode*);
    const PassNode& workPassNode() const;
    void            setWorkPassNode(PassNode*);
    void            markSelfWrite(TexNode*);
    void            markVirtualWrite(TexNode*);

private:
    TexNode* createNewTexNode(const TexNode::Desc&);

    RenderGraph& m_rg;
    PassNode*    m_passnode_wip { nullptr };
};

class RenderGraph {
public:
    RenderGraph();

    PassNode* getPassNode(NodeID) const;
    TexNode*  getTexNode(NodeID) const;
    Pass*     getPass(NodeID) const;

    // all render pass
    std::vector<NodeID>                topologicalOrder() const;
    std::vector<std::vector<TexNode*>> getLastReadTexs(Span<const NodeID>) const;

    void ToGraphviz(std::string_view path) const { m_dg.ToGraphviz(path); };

    template<typename CB>
    bool afterBuild(NodeID pass_node_id, CB&& callback) {
        auto* pass_node = getPassNode(pass_node_id);
        if (pass_node == nullptr) return false;
        RenderGraphBuilder builder(*this);
        builder.setWorkPassNode(pass_node);
        return callback(builder, *m_map_pass[pass_node_id]);
    };

    template<typename TPass, typename CB>
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
        if (type == PassNode::Type::Virtual) m_set_vitrual_passnode.insert(node->ID());
        return node;
    }

private:
    friend class RenderGraphBuilder;
    void markPassNode(NodeID);
    bool isPassNode(NodeID) const;

    DependencyGraph m_dg;
    Set<NodeID>     m_set_passnode;
    Set<NodeID>     m_set_vitrual_passnode;

    Map<std::string, NodeID> m_key_texnode;

    Map<NodeID, std::shared_ptr<Pass>> m_map_pass;
};

} // namespace rg
} // namespace wallpaper
