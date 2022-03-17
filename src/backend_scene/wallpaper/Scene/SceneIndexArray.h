#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include "Utils/span.hpp"

namespace wallpaper
{
	class SceneIndexArray {
		constexpr static size_t Unit_Byte_Size {sizeof(uint32_t)};
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

		void Assign(std::size_t index, Span<uint32_t> data)     { AssignSpan(index, data); }
		void AssignHalf(std::size_t index, Span<uint16_t> data) { AssignSpan(index, data); }

		// Get
		const uint32_t* Data() const { return m_pData; }
		std::size_t DataCount() const { return m_size; }
		std::size_t DataSizeOf() const { return m_size * Unit_Byte_Size; }

		std::size_t CapacityCount() const { return m_capacity; }
		std::size_t CapacitySizeof() const { return m_capacity * Unit_Byte_Size; }

		uint32_t ID() const { return m_id; }
		void SetID(uint32_t id) { m_id = id; }
	private:
		bool IncreaseCheckSet(size_t size);

		template <typename T>
		void AssignSpan(std::size_t index, Span<T> data) {
			using in_value_type = T;
			if(!IncreaseCheckSet((index + data.size()) * sizeof(in_value_type)))
				return;
			std::copy(data.begin(), data.end(), ((in_value_type*)m_pData) + index);
		}

		uint32_t* m_pData;
		std::size_t m_size;
		std::size_t m_capacity;

		uint32_t m_id;
	};
}
