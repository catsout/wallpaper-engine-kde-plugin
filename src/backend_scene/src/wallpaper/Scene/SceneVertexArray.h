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
		struct SceneVertexAttribute {
			std::string name;
			VertexType type;
			bool padding {true};
		};
		struct SceneVertexAttributeOffset {
			SceneVertexAttribute attr;
			uint offset;
		};

		SceneVertexArray(const std::vector<SceneVertexAttribute>& attrs, const std::size_t count);
		~SceneVertexArray();

		SceneVertexArray(SceneVertexArray&& other):m_pData(other.m_pData),
												  m_attributes(other.m_attributes),
												  m_oneSize(other.m_oneSize),
												  m_size(other.m_size),
												  m_capacity(other.m_capacity),
												  m_id(other.m_id) {
			other.m_pData = nullptr;
		}
		SceneVertexArray(const SceneVertexArray&) = delete;

		bool AddVertex(const float*);
		bool SetVertex(const std::string& name, const std::vector<float>& data);
		bool SetVertexs(std::size_t index, std::size_t count, const float* data);

		const float* Data() const { return m_pData; }
		std::size_t DataSize() const { return m_size; }
		std::size_t DataSizeOf() const { return m_size * sizeof(float); }
		std::size_t VertexCount() const { return m_size / m_oneSize; }
		std::size_t CapacitySize() const { return m_capacity; }
		std::size_t CapacitySizeOf() const { return m_capacity * sizeof(float); }
		std::size_t OneSize() const { return m_oneSize; }
		std::size_t OneSizeOf() const { return m_oneSize * sizeof(float); }

		const auto& Attributes() const { return m_attributes; }
		std::vector<SceneVertexAttributeOffset> GetAttrOffset() const;

		uint32_t ID() const { return m_id; }
		void SetID(uint32_t id) { m_id = id; }

		static uint8_t TypeCount(VertexType);
		static uint8_t RealAttributeSize(const SceneVertexAttribute&);
	private:
		const std::vector<SceneVertexAttribute> m_attributes;
		float* m_pData {nullptr};
		std::size_t m_oneSize {0};
		std::size_t m_size {0};
		std::size_t m_capacity {0};

		uint32_t m_id;
	};
}
