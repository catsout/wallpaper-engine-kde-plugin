#pragma once 
#include <cstdint>
#include <vector>
#include <stack>
#include <limits>
#include <cassert>

namespace wallpaper {

constexpr uint16_t Uninitialed { std::numeric_limits<uint16_t>::max() };
constexpr uint16_t HandleMaxNum { std::numeric_limits<uint16_t>::max() - 1 };


template <typename THandle>
struct Handle { uint32_t idx {0}; }; 

template <typename THandle>
inline bool HandleInitialed(THandle h) {
	return h.idx != Uninitialed;
}


constexpr uint16_t InvalidIdx {0};

template <typename THandle>
inline bool HandleValied(THandle h) {
	return h.idx != InvalidIdx;
}

/*
class THandleResource {
	class Desc {};
	THandleResource() {};
	static void Init(THandleResource&, const Desc&);
	static void Destroy(THandleResource&);
}
*/
template<typename THandleResource, typename THandle = Handle<THandleResource>>
class HandlePool {
public:
	static constexpr uint16_t InvalidIdx {0};
	static constexpr uint16_t Shift {16};

	HandlePool(uint16_t num)
		:m_size(num),
		m_freeIdxs(m_size),
		m_counts(m_size+1, 0),
		m_slots(m_size+1) {
		for(uint16_t i=0,v=num ; i < num; i++,v--) {
			m_freeIdxs[i] = v;
		}
	}
	~HandlePool() = default;
	HandlePool(const HandlePool&) = delete;
	HandlePool& operator=(const HandlePool&) = delete;

	THandle Alloc(const typename THandleResource::Desc& desc, bool import = false) {
		uint32_t idx = AllocIdx();
		if(idx == InvalidIdx) return { InvalidIdx };
		uint32_t count = ++m_counts[idx];
		auto& slot = m_slots.at(idx);
		slot.import = import;
		slot.id = (count << Shift)|(idx);
		//if(!import)
			THandleResource::Init(slot.rsc, desc);
		return { slot.id };
	};
	void Free(THandle h) {
		uint16_t idx = h.idx;
		FreeIdx(idx);
		//if(!m_slots.at(idx).import)
			THandleResource::Destroy(m_slots.at(idx).rsc);
		RestSlot(m_slots[idx]);
	}
	THandleResource* Lookup(THandle h) {
		if(h.idx == InvalidIdx) return nullptr;
		uint16_t idx = h.idx;
		auto& slot = m_slots.at(idx);
		if(h.idx == slot.id) {
			return &slot.rsc;
		}
		return nullptr;
	}

	std::size_t Size() const { return m_size; }

	void FreeAll() {
		m_counts.resize(m_size+1, 0);
		for(auto& el:m_slots) {
			if(el.id != 0) {
				THandleResource::Destroy(el.rsc);
			}
			RestSlot(el);
		}
	}

private:
	class Slot {
	public:
		Slot() = default;
		~Slot() = default;
		Slot(const Slot&) = delete;

		uint32_t id {0};
		bool import {false};
		THandleResource rsc;
	};

	uint16_t AllocIdx() {
		if(m_freeIdxs.empty()) return InvalidIdx;
		uint16_t top = m_freeIdxs.back();
		m_freeIdxs.pop_back();
		return top;
	}
	void FreeIdx(uint16_t idx) {
		if(m_size == m_freeIdxs.size()) return;
		m_freeIdxs.push_back(idx);
	}
	void RestSlot(Slot& slot) {
		slot.id = 0;
		slot.rsc = THandleResource();
	}
	
	std::size_t m_size;
	std::vector<uint16_t> m_freeIdxs;
	std::vector<uint16_t> m_counts;
	std::vector<Slot> m_slots;
};

}