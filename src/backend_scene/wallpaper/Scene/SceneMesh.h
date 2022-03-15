#pragma once
#include <vector>
#include <memory>
#include <cstddef>
#include <climits>

#include "SceneVertexArray.h"
#include "SceneIndexArray.h"
#include "SceneMaterial.h"

namespace wallpaper
{
class SceneMesh {
public:
	SceneMesh(bool dynamic = false):m_dynamic(dynamic),m_dirty(false),
		m_data(std::make_shared<Data>()) {}

	std::size_t VertexCount() const { return m_data->vertexArrays.size(); }
	std::size_t IndexCount() const { return m_data->indexArrays.size(); }

	MeshPrimitive Primitive() const { return m_primitive; }
	uint32_t PointSize() const { return m_pointSize; }
	bool Dynamic() const { return m_dynamic; }
	bool Dirty() const { return m_dirty; }
	void SetDirty() { m_dirty = true; }
	void SetClean() { m_dirty = false; }

	uint32_t ID() const { return m_id; };
	void SetID(uint32_t v) { m_id = v; };

	const SceneVertexArray& GetVertexArray(const std::size_t index) const { return m_data->vertexArrays[index]; }
	const SceneIndexArray& GetIndexArray(const std::size_t index) const { return m_data->indexArrays[index]; }	

	SceneVertexArray& GetVertexArray(const std::size_t index) { return m_data->vertexArrays[index]; }
	SceneIndexArray& GetIndexArray(const std::size_t index) { return m_data->indexArrays[index]; }	


	void AddIndexArray(SceneIndexArray&& array) {
		m_data->indexArrays.emplace_back(std::move(array));
	}
	void AddVertexArray(SceneVertexArray&& array) {
		m_data->vertexArrays.emplace_back(std::move(array));
	}
	void AddMaterial(SceneMaterial&& material) {
		m_material = std::make_shared<SceneMaterial>(material);
	}

	void SetPrimitive(MeshPrimitive v) {  m_primitive = v; }
	void SetPointSize(uint32_t v) { m_pointSize = v; }


	SceneMaterial* Material() { return m_material.get(); }

	void ChangeMeshDataFrom(const SceneMesh& o) {
		m_data = o.m_data;
	}

private:
	struct Data {
		std::vector<SceneVertexArray> vertexArrays;
		std::vector<SceneIndexArray> indexArrays;
	};

	uint32_t m_id { std::numeric_limits<uint32_t>::max() };
	MeshPrimitive m_primitive {MeshPrimitive::TRIANGLE};
	uint32_t m_pointSize {1};
	bool m_dynamic;
	bool m_dirty;

	std::shared_ptr<Data> m_data;
	std::shared_ptr<SceneMaterial> m_material;
};

}
