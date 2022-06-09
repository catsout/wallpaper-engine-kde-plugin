#include "WPParticleRawGener.h"
#include <cstring>
#include <Eigen/Dense>
#include <array>
#include "SpecTexs.hpp"
#include "ParticleModify.h"
#include "Utils/Logging.h"

using namespace wallpaper;
using namespace Eigen;

struct WPGOption {
    bool thick_format { false };
    bool geometry_shader { false };
};

namespace
{
inline void AssignVertexTimes(Span<float> dst, Span<const float> src, uint num) {
    const uint dst_one_size = dst.size() / num;
    for (uint i = 0; i < num; i++) {
        std::copy(src.begin(), src.end(), dst.begin() + i * dst_one_size);
    }
}

inline void AssignVertex(Span<float> dst, Span<const float> src, uint num) {
    const uint dst_one_size = dst.size() / num;
    const uint src_one_size = src.size() / num;
    for (uint i = 0; i < num; i++) {
        std::copy_n(src.begin() + i * src_one_size, src_one_size, dst.begin() + i * dst_one_size);
    }
}

inline size_t GenParticleData(Span<const Particle> particles, const ParticleRawGenSpecOp& specOp,
                              WPGOption opt, SceneVertexArray& sv) {
    std::array<float, 32 * 4> storage;

    float* data = storage.data();

    const auto oneSize = sv.OneSize();
    uint       i { 0 };
    for (const auto& p : particles) {
        float size = p.size / 2.0f;

        std::size_t offset = 0;

        float lifetime = p.lifetime;
        specOp(p, { &lifetime });

        // pos
        AssignVertexTimes({ data + offset, oneSize * 4 },
                          std::array { p.position[0], p.position[1], p.position[2] },
                          4);
        offset += 4;
        // TexCoordVec4
        float      rz = p.rotation[2];
        std::array t { 0.0f, 1.0f, rz, size, 1.0f, 1.0f, rz, size,
                       1.0f, 0.0f, rz, size, 0.0f, 0.0f, rz, size };
        AssignVertex({ data + offset, oneSize * 4 }, t, 4);
        offset += 4;

        // color
        AssignVertexTimes({ data + offset, oneSize * 4 },
                          std::array { p.color[0], p.color[1], p.color[2], p.alpha },
                          4);
        offset += 4;

        if (opt.thick_format) {
            AssignVertexTimes({ data + offset, oneSize * 4 },
                              std::array { p.velocity[0], p.velocity[1], p.velocity[2], lifetime },
                              4);
            offset += 4;
        }
        // TexCoordC2
        AssignVertexTimes(
            { data + offset, oneSize * 4 }, std::array { p.rotation[0], p.rotation[1] }, 4);

        sv.SetVertexs((i++) * 4, 4, data);
    }
    return particles.size();
}

inline size_t GenRopeParticleData(Span<const Particle>        particles,
                                  const ParticleRawGenSpecOp& specOp, WPGOption opt,
                                  SceneVertexArray& sv) {
    /*
    attribute vec4 a_PositionVec4;
    attribute vec4 a_TexCoordVec4;
    attribute vec4 a_TexCoordVec4C1;

    #if THICKFORMAT
    attribute vec4 a_TexCoordVec4C2;
    attribute vec4 a_TexCoordVec4C3;
    attribute vec2 a_TexCoordC4;
    #else
    attribute vec3 a_TexCoordVec3C2;
    attribute vec2 a_TexCoordC3;
    #endif

    attribute vec4 a_Color;

    #define in_ParticleTrailLength (a_TexCoordVec4.w)
    #define in_ParticleTrailPosition (a_TexCoordVec4C1.w)
    */
    std::array<float, 32 * 4> storage;

    float* data = storage.data();

    const auto one_size = sv.OneSize();
    uint       i { 0 };
    for (const auto& p : particles) {
        if (i == 0) {
            i++;
            continue;
        }
        if (! ParticleModify::LifetimeOk(p)) break;

        const auto& pre_p  = particles[i - 1];
        float       size   = p.size / 2.0f;
        std::size_t offset = 0;

        float lifetime = p.lifetime;
        specOp(p, { &lifetime });
        float in_ParticleTrailLength   = particles.size();
        float in_ParticleTrailPosition = i - 1;

        Vector3f cp_vec = AngleAxisf(p.rotation[2] + M_PI / 2.0f, Vector3f::UnitZ()) *
                          Vector3f { 0.0f, size / 2.0f, 0.0f };
        Vector3f pos_vec = Vector3f { p.position } - Vector3f { pre_p.position };

        cp_vec       = pos_vec.normalized().dot(cp_vec) > 0 ? cp_vec : -1.0f * cp_vec;
        auto&    sp  = pre_p;
        auto&    ep  = p;
        Vector3f scp = Vector3f { sp.position } + cp_vec;
        Vector3f ecp = Vector3f { ep.position } - cp_vec;

        // a_PositionVec4: start pos
        AssignVertexTimes({ data + offset, one_size * 4 },
                          std::array { sp.position[0], sp.position[1], sp.position[2], size },
                          4);
        offset += 4;
        // a_TexCoordVec4: end pos
        AssignVertexTimes(
            { data + offset, one_size * 4 },
            std::array { ep.position[0], ep.position[1], ep.position[2], in_ParticleTrailLength },
            4);
        offset += 4;

        // a_TexCoordVec4C1: cp start pos
        AssignVertexTimes({ data + offset, one_size * 4 },
                          std::array { scp[0], scp[1], scp[2], in_ParticleTrailPosition },
                          4);
        offset += 4;

        if (opt.thick_format) {
            // a_TexCoordVec4C2: cp end pos, size_end
            AssignVertexTimes(
                { data + offset, one_size * 4 }, std::array { ecp[0], ecp[1], ecp[2], size }, 4);
            offset += 4;
            // a_TexCoordVec4C3: color_end
            AssignVertexTimes({ data + offset, one_size * 4 },
                              std::array { p.color[0], p.color[1], p.color[2], p.alpha },
                              4);
            offset += 4;
            // a_TexCoordC4
            std::array t { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            AssignVertex({ data + offset, one_size * 4 }, t, 4);
            offset += 4;
        } else {
            // a_TexCoordVec3C2: cp end pos
            AssignVertexTimes(
                { data + offset, one_size * 4 }, std::array { ecp[0], ecp[1], ecp[2] }, 4);
            offset += 4;

            // a_TexCoordC3
            std::array t { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            AssignVertex({ data + offset, one_size * 4 }, t, 4);
            offset += 4;
        }

        // a_Color
        AssignVertexTimes({ data + offset, one_size * 4 },
                          std::array { p.color[0], p.color[1], p.color[2], p.alpha },
                          4);

        sv.SetVertexs((i++) * 4, 4, data);
    }
    return i == 0 ? 0 : i - 1;
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

    WPGOption opt;

    opt.thick_format = sv.GetOption(WE_CB_THICK_FORMAT);

    size_t particle_num { 0 };
    if (sv.GetOption(WE_PRENDER_ROPE))
        particle_num = GenRopeParticleData(particles, specOp, opt, sv);
    else
        particle_num = GenParticleData(particles, specOp, opt, sv);

    uint16_t indexNum = (si.DataCount() * 2) / 6;
    if (particle_num > indexNum) {
        updateIndexArray(indexNum, particles.size(), si);
        si.SetRenderDataCount(0);
    } else {
        si.SetRenderDataCount(particle_num * 6 / 2);
    }
}