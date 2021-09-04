#pragma once
#include <string>
#include <string_view>
#include "PassNode.h"
#include "DependencyGraph.h"
#include "Utils/Log.h"

namespace wallpaper {
namespace fg {

class FrameGraph;
class FrameGraphBuilder;

class ResourceNode : public DependencyGraph::Node {
	friend class FrameGraph;
	friend class FrameGraphBuilder;
public:
	ResourceNode() {}

	//std::string Name() const { return m_name; }

	bool IsFromMoved() const { return m_mover != nullptr && m_fromMoved; }
	bool IsToMoved() const { return m_mover != nullptr && !m_fromMoved; }

	bool IsWrited() const { return m_writer != nullptr; }
	PassNode* GetMover() const { return m_mover; }
	PassNode* GetWrite() const { return m_writer; }


	struct ResourceHandle { uint16_t idx {Uninitialed}; };
	template <typename TResourceHandle>
	void LinkResource(TResourceHandle h) {
		m_handle = { h.idx };
	}

	void SetName(std::string_view v) { m_name = v; }

	ResourceHandle GetResourceHandle() const { return m_handle; }

	std::string ToGraphviz() const override {
		auto sid = std::to_string(m_handle.idx);
		return GraphID() + "[label=\""+m_name+" "+sid+"\" shape=ellipse]";
	}
private:
	void SetMoved(PassNode* mover, bool fromMoved) { m_mover = mover; m_fromMoved = fromMoved; }
	PassNode* m_mover {nullptr};
	PassNode* m_writer {nullptr};
	bool m_fromMoved {false};

	std::string m_name;
	ResourceHandle m_handle;
};
}
}