#pragma once

#include "Interface/IGraphicManager.h"

namespace wallpaper {
namespace fg {

class FrameGraph;
class FrameGraphResourceManager;

class PassBase {
	friend class FrameGraph;
private:
	PassBase(const PassBase&) = delete;
	PassBase& operator=(const PassBase&) = delete;
protected:
	PassBase() = default;	
	virtual ~PassBase() = default;

	virtual void execute(FrameGraphResourceManager&) = 0;
};

class MovePass : public PassBase {
	friend class FrameGraph;
public:
private:
	MovePass() {}
	virtual ~MovePass() = default;
	virtual void execute(FrameGraphResourceManager&) {};
};

template<typename Data, typename Execute>
class Pass : public PassBase {
	friend class FrameGraph;
public:
	const Data& GetData() const { return m_data; }
	const Data* operator->() const { return &m_data; }
private:
	Pass(Execute&& e):m_execute(std::move(e)) {}
	virtual ~Pass() = default;

	void execute(FrameGraphResourceManager& rsm) override {
		m_execute(rsm, m_data);
	}

	Data m_data;
	Execute m_execute;
};
}
}