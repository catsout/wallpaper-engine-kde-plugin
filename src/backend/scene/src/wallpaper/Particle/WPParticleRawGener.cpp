#include "WPParticleRawGener.h"
#include <cstring>
#include "Log.h"

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

std::vector<float> WPParticleRawGener::GenGLData(const Particle& p, const SceneVertexArray& vertex) {
	std::size_t oneSize = vertex.OneSize();
	std::vector<float> result(oneSize * 4);

	float size = p.size/2.0f;

	std::size_t offset = 0;
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
			AssignVertexTimes(&result[0], {p.velocity[0], p.velocity[1], p.velocity[2], p.lifetime}, offset, oneSize, 4);
		} else if(el.name == "a_TexCoordC2") {
			AssignVertexTimes(&result[0], {p.rotation[0], p.rotation[1]}, offset, oneSize, 4);
		} else {
			//LOG_ERROR("unknown ...");
		}
		offset+=4;
	}
	return result;
}