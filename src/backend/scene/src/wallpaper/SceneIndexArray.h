#pragma once
#include <vector>
#include <cstdint>

namespace wallpaper
{
	class SceneIndexArray {
	public:
		SceneIndexArray(const uint32_t* data, const size_t count):m_pData(data),
																m_count(count) {
		};

		SceneIndexArray(const std::vector<uint32_t>& data);

		SceneIndexArray(SceneIndexArray&& other):m_pData(other.m_pData),
												m_count(other.m_count) {
			other.m_pData = nullptr;
		}
		SceneIndexArray(const SceneIndexArray&) = delete;
		~SceneIndexArray() {
			if(m_pData != nullptr)
				delete[] m_pData;
		}

		// Get
		size_t DataSize() const { return m_count * sizeof(uint32_t); }
		const uint32_t* Data() const { return m_pData; }
		size_t DataCount() const { return m_count; }

	private:
		const uint32_t* m_pData;
		const size_t m_count;
	};
}
