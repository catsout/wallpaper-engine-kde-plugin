#pragma once
#include "DependencyGraph.hpp"
#include <string>

namespace wallpaper
{
namespace rg
{

class TexNode;
class PassNode : public DependencyGraph::Node {
public:
    enum class Type {
        CustomShader,
        Copy
    };
    static PassNode* addPassNode(DependencyGraph& dg, Type type);

    Type type() const;
    std::string_view name() const;

    void setName(std::string_view);

    std::string ToGraphviz() const override; 


private:
    Type m_type;
    std::string m_name { "unknown pass" };
};
}
} 
