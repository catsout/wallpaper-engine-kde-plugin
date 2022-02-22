#include "PassNode.hpp"

using namespace wallpaper::rg;

PassNode* PassNode::addPassNode(DependencyGraph& dg, PassNode::Type type) {
    std::unique_ptr<PassNode> node = std::make_unique<PassNode>();
    node->m_type = type;
    PassNode* pNode = node.get();
    dg.AddNode(std::move(node));
    return pNode;
}

PassNode::Type PassNode::type() const { return m_type; }

std::string_view PassNode::name() const { return m_name; }

void PassNode::setName(std::string_view name) {
    m_name = name;
}


std::string PassNode::ToGraphviz() const {
    return GraphID() + "[label=\""+m_name+"\"]";
}