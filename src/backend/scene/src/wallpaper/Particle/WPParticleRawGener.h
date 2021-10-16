#pragma once
#include "Interface/IParticleRawGener.h"
#include <functional>

namespace wallpaper {

template <std::size_t T>
inline void AssignVertexTimes(float*dst, const std::array<float,T>& src, uint32_t doffset, uint32_t dsize, uint32_t num) {
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, src.data(), T*sizeof(float));
	}
}

inline void AssignVertex(float*dst, float* data, std::size_t dataSize, uint32_t doffset, uint32_t dsize, uint32_t ssize) {
	uint32_t num = dataSize / ssize;
	for(uint32_t i=0;i<num;i++) {
		std::memcpy(dst+doffset+i*dsize, &data[i*ssize], ssize*sizeof(float));
	}
}

class WPParticleRawGener : public IParticleRawGener {
public:
	WPParticleRawGener() {};
	virtual ~WPParticleRawGener() {};

	virtual void GenGLData(const std::vector<Particle>&, SceneMesh&, ParticleRawGenSpecOp&);


};

}