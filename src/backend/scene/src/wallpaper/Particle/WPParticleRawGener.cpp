#include "WPParticleRawGener.h"
#include <cstring>
#include "Log.h"
#include "ParticleModify.h"

using namespace wallpaper;

void AssignVertexTimes(float*dst, const std::vector<float>& src, uint32_t doffset, uint32_t dsize, uint32_t num) {
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, &src[0], src.size()*sizeof(float));
	}
}
void AssignVertex(float*dst, const std::vector<float>& src, uint32_t doffset, uint32_t dsize, uint32_t ssize) {
	uint32_t num = src.size() / ssize;
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, &src[i*ssize], ssize*sizeof(float));
	}
}

std::vector<float> GenSingleGLData(const Particle& p, const SceneVertexArray& vertex, ParticleRawGenSpecOp& specOp) {
	std::size_t oneSize = vertex.OneSize();
	std::vector<float> result(oneSize * 4);

	float size = p.size/2.0f;

	std::size_t offset = 0;

	float lifetime = p.lifetime;
	specOp(p, {&lifetime});

	for(const auto& el:vertex.Attributes()) {
		if(el.name == "a_Position") {
			AssignVertexTimes(&result[0], {p.position[0], -p.position[1], p.position[2]}, offset, oneSize, 4);
		} else if(el.name == "a_Color") {
			AssignVertexTimes(&result[0], {p.color[0], p.color[1], p.color[2], p.alpha}, offset, oneSize, 4);
		} else if(el.name == "a_TexCoordVec4") {
			float rz = p.rotation[2];
			std::vector<float> t = {
				0.0f, 0.0f, rz, size,
				1.0f, 0.0f, rz, size,
				1.0f, 1.0f, rz, size, 
				0.0f, 1.0f, rz, size
			};
			AssignVertex(&result[0], t, offset, oneSize, 4);
		} else if(el.name == "a_TexCoordVec4C1") {
			AssignVertexTimes(&result[0], {p.velocity[0], -p.velocity[1], p.velocity[2], lifetime}, offset, oneSize, 4);
		} else if(el.name == "a_TexCoordC2") {
			AssignVertexTimes(&result[0], {p.rotation[0], p.rotation[1]}, offset, oneSize, 4);
		} else {
			//LOG_ERROR("unknown ...");
		}
		offset+=4;
	}
	return result;
}

void updateIndexArray(std::size_t index, std::size_t count, SceneIndexArray& iarray) {
	std::vector<uint32_t> indexs;
	for(uint32_t i=index;i<count;i++) {
		uint32_t x = i*4;
		std::vector<uint32_t> t {
			x, x+1, x+3,
			x+1, x+2, x+3
		};
		indexs.insert(indexs.end(), t.begin(), t.end());
	}
	iarray.Assign(index*6, &indexs[0], indexs.size());
}

void WPParticleRawGener::GenGLData(const std::vector<Particle>& particles, SceneMesh& mesh, ParticleRawGenSpecOp& specOp) {
	auto& sv = mesh.GetVertexArray(0);
	auto& si = mesh.GetIndexArray(0);

	uint32_t i = 0;
	for(const auto& p:particles) {
		sv.SetVertexs((i++)*4, 4, &(GenSingleGLData(p, sv, specOp)[0]));
	}
	uint32_t indexNum = si.DataCount()/6;
	if(particles.size() > indexNum) {
		updateIndexArray(indexNum, particles.size(), si);
	}

}