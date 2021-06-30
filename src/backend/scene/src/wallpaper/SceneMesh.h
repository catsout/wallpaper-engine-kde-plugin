#pragma once
#include <vector>
#include <memory>
#include <cstddef>

#include "SceneVertexArray.h"
#include "SceneIndexArray.h"
#include "SceneMaterial.h"

namespace wallpaper
{
class SceneMesh {
public:
	SceneMesh(bool dynamic = false):m_dynamic(dynamic),m_dirty(false) {}

	std::size_t VertexCount() const { return m_vertexArrays.size(); }
	std::size_t IndexCount() const { return m_indexArrays.size(); }

	MeshPrimitive Primitive() const { return m_primitive; }
	uint32_t PointSize() const { return m_pointSize; }
	bool Dynamic() const { return m_dynamic; }
	bool Dirty() const { return m_dirty; }
	void SetDirty() { m_dirty = true; }
	void SetClean() { m_dirty = false; }

	uint32_t ID() const { return m_id; };
	void SetID(uint32_t v) { m_id = v; };

	const SceneVertexArray& GetVertexArray(const std::size_t index) const { return m_vertexArrays[index]; }
	const SceneIndexArray& GetIndexArray(const std::size_t index) const { return m_indexArrays[index]; }	

	SceneVertexArray& GetVertexArray(const std::size_t index) { return m_vertexArrays[index]; }
	SceneIndexArray& GetIndexArray(const std::size_t index) { return m_indexArrays[index]; }	


	void AddIndexArray(SceneIndexArray&& array) {
		m_indexArrays.emplace_back(std::move(array));
	}
	void AddVertexArray(SceneVertexArray&& array) {
		m_vertexArrays.emplace_back(std::move(array));
	}
	void AddMaterial(SceneMaterial&& material) {
		m_material = std::make_shared<SceneMaterial>(material);
	}

	void SetPrimitive(MeshPrimitive v) {  m_primitive = v; }
	void SetPointSize(uint32_t v) { m_pointSize = v; }


	SceneMaterial* Material() { return m_material.get(); }


	static void GenCardMesh(SceneMesh& mesh, const std::vector<int> size, bool autosize=false) {
		float left = -size[0]/2.0f;
		float right = size[0]/2.0f;
		float bottom = -size[1]/2.0f;
		float top = size[1]/2.0f;
		float z = 0.0f;
		std::vector<float> pos = {
			 left, bottom, z,
			 right, bottom, z,
			 right,  top, z,
			 left,  top, z,
		};
		std::vector<float> texCoord;
		float tw = 1.0f,th = 1.0f;
		if(autosize) {
			uint32_t x = 1,y = 1;
			while(x < size[0]) x*=2;	
			while(y < size[1]) y*=2;	
			tw = size[0] / (float)x;
			th = size[1] / (float)y;
		}
		texCoord = {
			 0.0f, 0.0f,
			 tw, 0.0f,
			 tw, th,
			 0.0f, th,
		};
		std::vector<uint32_t> indices = { 
			0, 1, 3,
			1, 2, 3
		};
		SceneVertexArray vertex({
			{"a_Position", VertexType::FLOAT3},
			{"a_TexCoord", VertexType::FLOAT2},
		}, 4);
		vertex.SetVertex("a_Position", pos);
		vertex.SetVertex("a_TexCoord", texCoord);
		mesh.AddVertexArray(std::move(vertex));
		mesh.AddIndexArray(SceneIndexArray(indices));
	}


private:
	uint32_t m_id;
	MeshPrimitive m_primitive {MeshPrimitive::TRIANGLE};
	uint32_t m_pointSize {1};
	bool m_dynamic;
	bool m_dirty;
	std::vector<SceneVertexArray> m_vertexArrays;
	std::vector<SceneIndexArray> m_indexArrays;

	std::shared_ptr<SceneMaterial> m_material;
};

}
