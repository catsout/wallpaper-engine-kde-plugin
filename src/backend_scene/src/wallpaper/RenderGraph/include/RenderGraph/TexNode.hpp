#pragma once
#include "DependencyGraph.hpp"
#include <string>

namespace wallpaper
{
namespace rg
{

class RenderGraphBuilder;
class PassNode;

class TexNode : public DependencyGraph::Node {
public:
    enum class TexType {
        Imported,
        Temp
    };
    struct Desc {
        std::string name;
        std::string key;
        TexType type;
    };
    static TexNode* addTexNode(DependencyGraph& dg, const Desc& type);
    static TexNode* addNewVersion(DependencyGraph& dg, TexNode* pre);

    TexType type() const;
    std::string_view name() const;
    std::string_view key() const;
    size_t version() const;
    PassNode* writer() const;
    Desc genDesc() const;

    TexNode* preVer() const;
    TexNode* nextVer() const;

    void setName(std::string_view);
    void setKey(std::string_view);
    void setWriter(PassNode*);

    std::string ToGraphviz() const override; 
private:
    friend class RenderGraphBuilder;
    TexType m_type;
    std::string m_key;
    std::string m_name {"unknown tex"};

    size_t    m_version {0};
    TexNode*  m_pre    {nullptr};
    TexNode*  m_next    {nullptr};
    PassNode* m_writer  {nullptr};
};
}
} 
