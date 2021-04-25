#pragma once
#include <vector>
#include <string>
#include <cstddef>

namespace wallpaper
{
	enum class VertexType {
		FLOAT1,
		FLOAT2,
		FLOAT3,
		FLOAT4
	};

	class SceneVertexArray {
	public:
		SceneVertexArray(const char* attr, const float* data, const std::size_t count, const VertexType type):m_attribute(attr),
	m_pData(data),
	m_count(count),
	m_type(type) {};

		SceneVertexArray(const char* attr, const std::vector<float>& data, const VertexType type);

		SceneVertexArray(SceneVertexArray&& other):m_pData(other.m_pData),
												  m_count(other.m_count),
												  m_type(other.m_type),
												  m_attribute(std::move(other.m_attribute)) {
			other.m_pData = nullptr;
		}
		SceneVertexArray(const SceneVertexArray&) = delete;

		~SceneVertexArray() {
			if(m_pData != nullptr)
				delete[] m_pData;
		}
		// Get
		const std::string& Attribute() const { return m_attribute; }	
		std::size_t DataSize() const { return m_count * sizeof(float); }
		const float* Data() const { return m_pData; }
		std::size_t DataCount() const { return m_count; }
		uint8_t TypeCount() const {
			uint8_t type = (uint8_t)m_type;
			uint8_t first = (uint8_t)VertexType::FLOAT1;
			return (type - first + 1);
		}

	private:
		const std::string m_attribute;
		const float* m_pData;
		const std::size_t m_count;
		const VertexType m_type;
	};
}
