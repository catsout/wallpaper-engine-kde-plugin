#include "WPParticleRawGener.h"
#include <cstring>
#include <Eigen/Dense>
#include <array>
#include "SpecTexs.hpp"
#include "ParticleModify.h"
#include "Utils/Logging.h"

using namespace wallpaper;
using namespace Eigen;
namespace
{
inline void GenSingleGLData(const Particle& p, size_t oneSize, const ParticleRawGenSpecOp& specOp,
                            float* data, bool hasTexCoordVec4C1) {
    float size = p.size / 2.0f;

    std::size_t offset = 0;

    float lifetime = p.lifetime;
    specOp(p, { &lifetime });

    // pos
    AssignVertexTimes(data,
                      std::array<float, 3> { p.position[0], p.position[1], p.position[2] },
                      offset,
                      oneSize,
                      4);
    offset += 4;
    // TexCoordVec4
    float rz = p.rotation[2];
    float t[16] { 0.0f, 1.0f, rz, size, 1.0f, 1.0f, rz, size,
                  1.0f, 0.0f, rz, size, 0.0f, 0.0f, rz, size };
    AssignVertex(data, t, 16, offset, oneSize, 4);
    offset += 4;

    // color
    AssignVertexTimes(data,
                      std::array<float, 4> { p.color[0], p.color[1], p.color[2], p.alpha },
                      offset,
                      oneSize,
                      4);
    offset += 4;

    if (hasTexCoordVec4C1) {
        AssignVertexTimes(
            data,
            std::array<float, 4> { p.velocity[0], p.velocity[1], p.velocity[2], lifetime },
            offset,
            oneSize,
            4);
        offset += 4;
    }
    // TexCoordC2
    AssignVertexTimes(
        data, std::array<float, 2> { p.rotation[0], p.rotation[1] }, offset, oneSize, 4);
}

inline void updateIndexArray(uint16_t index, size_t count, SceneIndexArray& iarray) {
    constexpr size_t single_size = 6;
    const uint16_t   cv          = index * 4;

    std::array<uint16_t, single_size> single;
    // 0 1 3
    // 1 2 3
    single[0] = cv;
    single[1] = cv + 1;
    single[2] = cv + 3;
    single[3] = cv + 1;
    single[4] = cv + 2;
    single[5] = cv + 3;
    // every particle
    for (uint16_t i = index; i < count; i++) {
        iarray.AssignHalf(i * single_size, single);
        for (auto& x : single) x += 4;
    }
}
} // namespace

void WPParticleRawGener::GenGLData(const std::vector<Particle>& particles, SceneMesh& mesh,
                                   ParticleRawGenSpecOp& specOp) {
    auto& sv = mesh.GetVertexArray(0);
    auto& si = mesh.GetIndexArray(0);

    uint32_t i = 0;

    bool hasTexCoordVec4C1 { false };
    for (const auto& el : sv.Attributes()) {
        if (el.name == WE_IN_TEXCOORDVEC4C1) hasTexCoordVec4C1 = true;
    }
    std::array<float, 32 * 4> storage;

    float* pStorage = storage.data();
    auto   oneSize  = sv.OneSize();
    for (const auto& p : particles) {
        GenSingleGLData(p, oneSize, specOp, pStorage, hasTexCoordVec4C1);
        sv.SetVertexs((i++) * 4, 4, pStorage);
    }
    uint16_t indexNum = (si.DataCount() * 2) / 6;
    if (particles.size() > indexNum) {
        updateIndexArray(indexNum, particles.size(), si);
    }
}