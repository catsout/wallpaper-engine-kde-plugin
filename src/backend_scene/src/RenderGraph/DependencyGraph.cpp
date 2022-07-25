#include "DependencyGraph.hpp"
#include <algorithm>
#include <functional>
#include <fstream>
#include <cassert>
#include "Utils/Logging.h"

using namespace wallpaper::rg;

std::vector<NodeID> DependencyGraph::GetNodeOut(NodeID i) const {
    const auto&         nexts = m_nodeNext[i];
    std::vector<NodeID> result(nexts.size());
    std::transform(nexts.begin(), nexts.end(), result.begin(), [](NodeID i) {
        return i;
    });
    return result;
}

std::vector<NodeID> DependencyGraph::GetNodeIn(NodeID i) const {
    std::vector<NodeID> result;
    for (NodeID in = 0; in < NodeNum(); in++) {
        for (NodeID j : m_nodeNext[in]) {
            if (i == j) result.push_back(in);
        }
    }
    return result;
}

NodeID DependencyGraph::AddNode(std::unique_ptr<Node>&& node) {
    m_nodes.emplace_back(std::move(node));
    m_nodeNext.push_back({});
    Node& n = *(m_nodes.back());
    n.id    = m_nodes.size() - 1;
    return n.id;
}
void DependencyGraph::Connect(NodeID n1, NodeID n2) { m_nodeNext[n1].insert(n2); }

typedef std::function<std::vector<NodeID>(NodeID)> NextNodeOp;
typedef std::function<void(NodeID)>                DfsCallbackOp;
static void Dfs(NodeID id, std::vector<bool>& rec, const NextNodeOp& next,
                const DfsCallbackOp& dcOp, const DfsCallbackOp& reverseOp) {
    if (rec[id]) return;
    rec[id] = true;
    if (dcOp) dcOp(id);
    for (const auto& el : next(id)) Dfs(el, rec, next, dcOp, reverseOp);
    if (reverseOp) reverseOp(id);
}

std::vector<NodeID> DependencyGraph::TopologicalOrder() const {
    using namespace std::placeholders;
    std::vector<NodeID> result;
    result.reserve(m_nodes.size());
    std::vector<bool> dfsrec(m_nodes.size(), false);
    for (usize i = 0; i < dfsrec.size(); i++) {
        if (dfsrec[i]) continue;
        Dfs(
            i,
            dfsrec,
            [this](NodeID i) {
                return GetNodeOut(i);
            },
            DfsCallbackOp(),
            [&](NodeID i) {
                result.push_back(i);
            });
    }
    std::reverse(result.begin(), result.end());
    return result;
}

void DependencyGraph::ToGraphviz(std::string_view path) const {
    std::string output;
    output.reserve(4096);
    std::ofstream fs;
    fs.open(std::string(path), std::fstream::out | std::fstream::trunc);
    if (! fs.is_open()) return;

    output += R"(
digraph framegraph {
node [shape=box]
)";
    for (const auto& n : m_nodes) {
        output += n->ToGraphviz();
        output += '\n';
    }
    for (usize i = 0; i < m_nodeNext.size(); i++) {
        for (const auto& e : m_nodeNext[i]) {
            output += "n" + std::to_string(i) + "->n" + std::to_string(e) + "\n";
        }
    }

    output += "}";

    fs << output;
    fs.close();
};
