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
	// temp
	uint32_t vao;

	std::size_t VertexCount() const { return m_vertexArrays.size(); }
	std::size_t IndexCount() const { return m_indexArrays.size(); }

	const SceneVertexArray& GetVertexArray(const std::size_t index) const { return m_vertexArrays[index]; }
	const SceneIndexArray& GetIndexArray(const std::size_t index) const { return m_indexArrays[index]; }	

	void AddIndexArray(SceneIndexArray&& array) {
		m_indexArrays.emplace_back(std::move(array));
	}
	void AddVertexArray(SceneVertexArray&& array) {
		m_vertexArrays.emplace_back(std::move(array));
	}
	void AddMaterial(SceneMaterial&& material) {
		m_material = std::make_shared<SceneMaterial>(material);
	}

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
		mesh.AddVertexArray(SceneVertexArray("a_Position", pos, VertexType::FLOAT3));
		mesh.AddVertexArray(SceneVertexArray("a_TexCoord", texCoord, VertexType::FLOAT2));
		mesh.AddIndexArray(SceneIndexArray(indices));
	}


private:
	std::vector<SceneVertexArray> m_vertexArrays;
	std::vector<SceneIndexArray> m_indexArrays;

	std::shared_ptr<SceneMaterial> m_material;
};

}
