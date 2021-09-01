#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "DependencyGraph.h"

namespace wallpaper {
namespace fg {

class Resource {
public:
	Resource() = default;
	virtual ~Resource() = default;

	virtual void Initialize() = 0; 
	virtual bool Initialed() const = 0;
};

template <typename TResourceIn>
class ResourceContainer {
public:
	struct ResourceHandle { uint16_t idx {Uninitialed}; };

	ResourceHandle New(TResourceIn res, NodeID nodeId) {
		const auto& name = res.desc.name;
		if(m_nameMap.count(name) > 0) {
			auto index = static_cast<uint16_t>(m_nameMap.at(name));
			m_resources.at(index).linkedNodes.insert(nodeId);
			return { index };
		} 
		auto index = static_cast<uint16_t>(m_resources.size());
		m_resources.push_back({{nodeId}, res});
		m_nameMap[name] = index;
		return { index };
	}
	bool Check(ResourceHandle h, NodeID id) {
		if(h.idx < m_resources.size()) {
			return m_resources.at(h.idx).linkedNodes.count(id) > 0;
		}
		return false;
	}
	TResourceIn* Get(ResourceHandle h) {
		if(h.idx >= m_resources.size()) return nullptr;
		return &m_resources.at(h.idx).resource; 
	}
	std::vector<NodeID> GetLinkedNodes(ResourceHandle h) {
		if(h.idx >= m_resources.size()) {
			return {};
		}
		return m_resources.at(h.idx).linkedNodes;
	}
	void AddLinkedNode(ResourceHandle h, NodeID id) {
		if(h.idx >= m_resources.size()) {
			return;
		}
		m_resources.at(h.idx).linkedNodes.insert(id);
	}
private:
	struct One {
		std::unordered_set<NodeID> linkedNodes;	
		TResourceIn resource;
	};
	std::vector<One> m_resources;

	std::unordered_map<std::string, size_t> m_nameMap;
};
}
}