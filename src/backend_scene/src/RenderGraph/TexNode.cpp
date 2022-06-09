#include "TexNode.hpp"
#include "PassNode.hpp"

using namespace wallpaper::rg;

TexNode* TexNode::addTexNode(DependencyGraph& dg, const Desc& desc) {
    std::unique_ptr<TexNode> node = std::make_unique<TexNode>();
    node->m_type = desc.type;
    node->m_name = desc.name;
    node->m_key = desc.key;
    TexNode* pNode = node.get();
    dg.AddNode(std::move(node));
    return pNode;
}


TexNode* TexNode::addNewVersion(DependencyGraph& dg, TexNode* pre) {
    TexNode* node = addTexNode(dg, pre->genDesc());
    node->m_version = pre->m_version+1;

    pre->m_next = node;
    node->m_pre = pre;

    return node;
}


TexNode::Desc TexNode::genDesc() const {
    return Desc{
        .name = m_name,
        .key = m_key,
        .type = m_type
    };
}


TexNode::TexType TexNode::type() const { return m_type; }
std::string_view TexNode::name() const { return m_name; };
std::string_view TexNode::key() const { return m_key; }
size_t TexNode::version() const { return m_version; } ;
PassNode* TexNode::writer() const { return m_writer; };
TexNode* TexNode::preVer() const { return m_pre; };
TexNode* TexNode::nextVer() const { return m_next; };


void TexNode::setName(std::string_view in) {
    m_name = in;
}
void TexNode::setKey(std::string_view in) {
    m_key = in;
}
void TexNode::setWriter(PassNode* w) {
    m_writer = w;
}


std::string TexNode::ToGraphviz() const {
    return GraphID() + "[label=\"" +m_key+ " v:" + std::to_string(m_version) + "\" shape=ellipse]";
}