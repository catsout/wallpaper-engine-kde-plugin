#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include "Handle.h"

namespace wallpaper {
namespace fg {

class FrameGraphBuilder;
class FrameGraph;
class ResourceOper {
	friend class FrameGraphBuilder;
	friend class FrameGraph;
public:
	struct NodeIndex { uint16_t idx {Uninitialed}; };
	
	uint16_t Index() const { return m_nodeIndex.idx; }
	bool IsInitialed() const { return HandleInitialed(m_nodeIndex); }
protected:
	ResourceOper() {};
	virtual ~ResourceOper() = default;
protected:
	void Initialize(uint16_t index) { m_nodeIndex.idx = index; }
private:
	NodeIndex m_nodeIndex;
};


class FrameGraphMutableResource : public ResourceOper {
public:
	FrameGraphMutableResource() = default;
	~FrameGraphMutableResource() = default;
};

class FrameGraphResource : public ResourceOper {
public:
	FrameGraphResource() = default;
	~FrameGraphResource() = default;

	FrameGraphResource(const FrameGraphMutableResource& mres) {
		Initialize(mres.Index());
	}
	FrameGraphResource& operator=(const FrameGraphMutableResource& mres) {
		Initialize(mres.Index());
		return *this;
	}

};

}
}