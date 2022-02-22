#pragma once
#include <cstdint>
#include <vector>
#include <numeric>
#include <memory>
#include <unordered_set>
#include <string>
#include <string_view>

namespace wallpaper {
namespace fg {

typedef size_t NodeID;

class DependencyGraph {
public:
	class Node {
		friend class DependencyGraph;
	public:
		Node() = default;
		virtual ~Node() = default;
		Node(const Node&) = delete;
		const Node& operator=(Node&) = delete;

		NodeID ID() const { return id; }	
		std::string GraphID() const { return "n" + std::to_string(id); }
		virtual std::string ToGraphviz() const {
			auto sid = std::to_string(id);
			return "n" + sid + "[label="+sid+"]";
		};
	private:
		NodeID id;
	};

	DependencyGraph() = default;
	~DependencyGraph() = default;
	DependencyGraph(const DependencyGraph&) = delete;
	DependencyGraph& operator=(const DependencyGraph&) = delete;
	DependencyGraph(DependencyGraph&& o):m_nodeNext(std::move(o.m_nodeNext)),
		m_nodes(std::move(o.m_nodes)) {}
	DependencyGraph& operator=(DependencyGraph&& o) {
		m_nodeNext = std::move(o.m_nodeNext);
		m_nodes = std::move(o.m_nodes);
		return *this;
	}

	size_t NodeNum() const { return m_nodes.size(); }
	size_t EdgeNum() const { return std::accumulate(m_nodeNext.begin(), m_nodeNext.end(), 
		0, 
		[](size_t sum, const auto& v) { return sum + v.size(); }); 
	}
	const Node& GetNode(NodeID i) const { return *m_nodes[i]; }
	Node* GetNode(NodeID i) { return m_nodes[i].get(); }

	std::vector<NodeID> GetNodeOut(NodeID) const;
	std::vector<NodeID> GetNodeIn(NodeID) const;

	NodeID AddNode(std::unique_ptr<Node>&&);
	void Connect(NodeID, NodeID);

	bool HasCycle() const;
	std::vector<NodeID> TopologicalOrder() const;

	void ToGraphviz(std::string_view) const;
private:
	std::vector<std::unordered_set<NodeID>> m_nodeNext;
	std::vector<std::unique_ptr<Node>> m_nodes;
};
}
}