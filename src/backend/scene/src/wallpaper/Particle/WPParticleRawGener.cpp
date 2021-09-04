#include "WPParticleRawGener.h"
#include <cstring>
#include <Eigen/Dense>
#include <array>
#include "ParticleModify.h"

using namespace wallpaper;
using namespace Eigen;

void AssignVertexTimes(float*dst, const std::array<float,2>& src, uint32_t doffset, uint32_t dsize, uint32_t num) {
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, src.data(), 2*sizeof(float));
	}
}
void AssignVertexTimes(float*dst, const std::array<float,3>& src, uint32_t doffset, uint32_t dsize, uint32_t num) {
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, src.data(), 3*sizeof(float));
	}
}
void AssignVertexTimes(float*dst, const std::array<float,4>& src, uint32_t doffset, uint32_t dsize, uint32_t num) {
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, src.data(), 4*sizeof(float));
	}
}

void AssignVertex(float*dst, float* data, std::size_t dataSize, uint32_t doffset, uint32_t dsize, uint32_t ssize) {
	uint32_t num = dataSize / ssize;
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, &data[i*ssize], ssize*sizeof(float));
	}
}

void GenSingleGLData(const Particle& p, const SceneVertexArray& vertex, ParticleRawGenSpecOp& specOp, float* data, bool hasTexCoordVec4C1) {
	std::size_t oneSize = vertex.OneSize();

	float size = p.size/2.0f;

	std::size_t offset = 0;

	float lifetime = p.lifetime;
	specOp(p, {&lifetime});
	
	// pos
	AssignVertexTimes(data, std::array<float,3>{p.position[0], -p.position[1], p.position[2]}, offset, oneSize, 4);
	offset+=4;
	// TexCoordVec4
	float rz = p.rotation[2];
	float t[16] {
		0.0f, 0.0f, rz, size,
		1.0f, 0.0f, rz, size,
		1.0f, 1.0f, rz, size, 
		0.0f, 1.0f, rz, size
	};
	AssignVertex(data, t, 16, offset, oneSize, 4);
	offset+=4;

	// color
	AssignVertexTimes(data, std::array<float,4>{p.color[0], p.color[1], p.color[2], p.alpha}, offset, oneSize, 4);
	offset+=4;


	if(hasTexCoordVec4C1) {
		AssignVertexTimes(data, std::array<float,4>{p.velocity[0], -p.velocity[1], p.velocity[2], lifetime}, offset, oneSize, 4);
		offset+=4;
	}
	// TexCoordC2
	AssignVertexTimes(data, std::array<float,2>{p.rotation[0], p.rotation[1]}, offset, oneSize, 4);
}

void updateIndexArray(std::size_t index, std::size_t count, SceneIndexArray& iarray) {
	std::vector<uint32_t> indexs;
	indexs.reserve(16);
	for(uint32_t i=index;i<count;i++) {
		uint32_t x = i*4;
		std::array<uint32_t, 6> t {
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

	bool hasTexCoordVec4C1 {false};
	for(const auto& el:sv.Attributes()) {
		if(el.name == "a_TexCoordVec4C1") hasTexCoordVec4C1 = true;
	}	
	std::vector<float> storage(sv.OneSize()*4);
	float* pStorage = &storage[0];
	for(const auto& p:particles) {
		GenSingleGLData(p, sv, specOp, pStorage, hasTexCoordVec4C1);
		sv.SetVertexs((i++)*4, 4, pStorage);
	}
	uint32_t indexNum = si.DataCount()/6;
	if(particles.size() > indexNum) {
		updateIndexArray(indexNum, particles.size(), si);
	}

}