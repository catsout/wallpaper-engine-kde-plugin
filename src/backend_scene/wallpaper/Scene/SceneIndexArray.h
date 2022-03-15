#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include "Utils/span.hpp"

namespace wallpaper
{
	class SceneIndexArray {
	public:
		SceneIndexArray(std::size_t indexCount);
		SceneIndexArray(Span<uint32_t> data);

		SceneIndexArray(SceneIndexArray&& other):m_pData(other.m_pData),
												m_size(other.m_size),
												m_capacity(other.m_capacity),
												m_id(other.m_id) {
			other.m_pData = nullptr;
		}
		SceneIndexArray(const SceneIndexArray&) = delete;
		~SceneIndexArray() {
			if(m_pData != nullptr)
				delete[] m_pData;
		}

		void Assign(std::size_t index, const uint32_t* data, std::size_t count);

		// Get
		std::size_t DataSizeOf() const { return m_size * sizeof(uint32_t); }
		const uint32_t* Data() const { return m_pData; }
		std::size_t DataCount() const { return m_size; }
		std::size_t CapacityCount() const { return m_capacity; }
		std::size_t CapacitySizeof() const { return m_capacity * sizeof(float); }

		uint32_t ID() const { return m_id; }
		void SetID(uint32_t id) { m_id = id; }
	private:
		uint32_t* m_pData;
		std::size_t m_size;
		std::size_t m_capacity;

		uint32_t m_id;
	};
}
