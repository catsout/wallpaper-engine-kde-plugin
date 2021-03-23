#pragma once
#include <vector>
#include "SceneVertexArray.h"
#include "SceneIndexArray.h"

namespace wallpaper
{
class SceneMesh {
	public:
		// temp use
		uint32_t vao = 0;	
		
		size_t VertexCount() const { return m_vertexArrays.size(); }
		size_t IndexCount() const { return m_indexArrays.size(); }

		const SceneVertexArray& GetVertexArray(const size_t index) const { return m_vertexArrays[index]; }
		const SceneIndexArray& GetIndexArray(const size_t index) const { return m_indexArrays[index]; }	

		void AddIndexArray(SceneIndexArray&& array) {
			m_indexArrays.emplace_back(std::move(array));
		}
		void AddVertexArray(SceneVertexArray&& array) {
			m_vertexArrays.emplace_back(std::move(array));
		}
		static void GenCardMesh(SceneMesh& mesh, const std::vector<int> size) {
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
			std::vector<float> texCoord = {
				 0.0f, 0.0f,
				 1.0f, 0.0f,
				 1.0f, 1.0f,
				 0.0f, 1.0f,
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
};

}
