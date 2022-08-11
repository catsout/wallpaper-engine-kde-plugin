#pragma once
#include <cstdint>
#include <vector>
#include <numeric>
#include <memory>
#include <unordered_set>
#include <string>
#include <string_view>

#include "Core/Literals.hpp"
#include "Core/NoCopyMove.hpp"

namespace wallpaper
{
namespace rg
{

typedef size_t NodeID;

class DependencyGraph : NoCopy {
public:
    class Node : NoCopy {
        friend class DependencyGraph;

    public:
        Node()          = default;
        virtual ~Node() = default;

        NodeID              ID() const { return id; }
        std::string         GraphID() const { return "n" + std::to_string(id); }
        virtual std::string ToGraphviz() const {
            auto sid = std::to_string(id);
            return "n" + sid + "[label=" + sid + "]";
        };

    private:
        NodeID id;
    };

    DependencyGraph()  = default;
    ~DependencyGraph() = default;

    DependencyGraph(DependencyGraph&& o)
        : m_nodeNext(std::move(o.m_nodeNext)), m_nodes(std::move(o.m_nodes)) {}
    DependencyGraph& operator=(DependencyGraph&& o) {
        m_nodeNext = std::move(o.m_nodeNext);
        m_nodes    = std::move(o.m_nodes);
        return *this;
    }

    size_t NodeNum() const { return m_nodes.size(); }
    size_t EdgeNum() const {
        return std::accumulate(
            m_nodeNext.begin(), m_nodeNext.end(), 0u, [](size_t sum, const auto& v) {
                return sum + v.size();
            });
    }
    // const Node& GetNode(NodeID i) const { return *m_nodes[i]; }
    Node* GetNode(NodeID i) const { return m_nodes[i].get(); }

    std::vector<NodeID> GetNodeOut(NodeID) const;
    std::vector<NodeID> GetNodeIn(NodeID) const;

    NodeID AddNode(std::unique_ptr<Node>&&);
    void   Connect(NodeID, NodeID);

    bool                HasCycle() const;
    std::vector<NodeID> TopologicalOrder() const;

    void ToGraphviz(std::string_view) const;

private:
    std::vector<std::unordered_set<NodeID>> m_nodeNext;
    std::vector<std::unique_ptr<Node>>      m_nodes;
};
} // namespace rg
} // namespace wallpaper
