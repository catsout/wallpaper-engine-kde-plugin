#include "WPMdlParser.hpp"
#include "Fs/VFS.h"
#include "Fs/IBinaryStream.h"
#include "WPCommon.hpp"
#include "Utils/Logging.h"
#include "Scene/SceneMesh.h"
#include "SpecTexs.hpp"
#include "wpscene/WPMaterial.h"
#include "WPShaderParser.hpp"

using namespace wallpaper;

namespace
{

WPPuppet::PlayMode ToPlayMode(std::string_view m) {
    if (m == "loop" || m.empty()) return WPPuppet::PlayMode::Loop;
    if (m == "mirror") return WPPuppet::PlayMode::Mirror;
    if (m == "single") return WPPuppet::PlayMode::Single;

    LOG_ERROR("unknown puppet animation play mode \"%s\"", m.data());
    assert(m == "loop");
    return WPPuppet::PlayMode::Loop;
}
} // namespace

// bytes * size
constexpr uint32_t singile_vertex  = 4 * (3 + 4 + 4 + 2);
constexpr uint32_t singile_indices = 2 * 3;

constexpr uint32_t singile_bone_frame = 4 * 9;

bool WPMdlParser::Parse(std::string_view path, fs::VFS& vfs, WPMdl& mdl) {
    auto pfile = vfs.Open("/assets/" + std::string(path));
    if (! pfile) return false;
    auto& f = *pfile;

    mdl.mdlv           = ReadMDLVesion(f);
    int32_t num_uknown = f.ReadInt32();
    int32_t num1       = f.ReadInt32();
    int32_t num2       = f.ReadInt32();
    mdl.mat_json_file  = f.ReadStr();
    // 0
    f.ReadInt32();

    uint32_t vertex_size = f.ReadUint32();
    if (vertex_size % singile_vertex != 0) {
        LOG_ERROR("unsupport mdl vertex size %d", vertex_size);
        return false;
    }

    uint32_t vertex_num = vertex_size / singile_vertex;
    mdl.vertexs.resize(vertex_num);
    for (auto& vert : mdl.vertexs) {
        for (auto& v : vert.position) v = f.ReadFloat();
        for (auto& v : vert.blend_indices) v = f.ReadUint32();
        for (auto& v : vert.weight) v = f.ReadFloat();
        for (auto& v : vert.texcoord) v = f.ReadFloat();
    }

    uint32_t indices_size = f.ReadUint32();
    if (indices_size % singile_indices != 0) {
        LOG_ERROR("unsupport mdl indices size %d", indices_size);
        return false;
    }

    uint32_t indices_num = indices_size / singile_indices;
    mdl.indices.resize(indices_num);
    for (auto& id : mdl.indices) {
        for (auto& v : id) v = f.ReadUint16();
    }

    mdl.mdls = ReadMDLVesion(f);
    // unknown
    f.ReadInt32();

    uint16_t bones_num = f.ReadUint16();
    // 1 byte
    f.ReadUint8();

    mdl.puppet  = std::make_shared<WPPuppet>();
    auto& bones = mdl.puppet->bones;
    auto& anims = mdl.puppet->anims;

    // uint16 unk
    // int32
    // int32
    // int32 size
    // mat4x3 bnoes
    // vec3 bone pos
    // int32
    bones.resize(bones_num);
    for (uint i = 0; i < bones_num; i++) {
        auto&       bone      = bones[i];
        std::string unk_extra = f.ReadStr();
        f.ReadUint8();
        int32_t unk2 = f.ReadInt32();

        bone.parent = f.ReadUint32();
        assert(bone.parent < i || bone.noParent());
        if (bone.parent >= i && ! bone.noParent()) {
            LOG_ERROR("mdl wrong bone parent index %d", bone.parent);
            return false;
        }

        uint32_t size = f.ReadUint32();
        if (size != 64) {
            LOG_ERROR("mdl unsupport bones size: %d", size);
            return false;
        }
        for (auto row : bone.transform.matrix().colwise()) {
            for (auto& x : row) x = f.ReadFloat();
        }
        /*
        auto trans = bone.transform.translation();
        LOG_INFO("trans: %f %f %f", trans[0], trans[1], trans[2]);
        */
    }

    // 1 byte
    f.ReadUint8();

    mdl.mdla = ReadMDLVesion(f);
    if (mdl.mdla != 0) {
        f.ReadInt32();
        uint anim_num = f.ReadUint32();
        anims.resize(anim_num);
        for (auto& anim : anims) {
            anim.id = f.ReadInt32();
            if (anim.id <= 0) {
                LOG_ERROR("wrong anime id %d", anim.id);
                return false;
            }
            f.ReadInt32();
            anim.name   = f.ReadStr();
            anim.mode   = ToPlayMode(f.ReadStr());
            anim.fps    = f.ReadFloat();
            anim.length = f.ReadUint32();
            f.ReadInt32();

            uint32_t b_num = f.ReadUint32();
            anim.bframes_array.resize(b_num);
            for (auto& bframes : anim.bframes_array) {
                f.ReadInt32();
                uint32_t byte_size = f.ReadUint32();
                uint32_t num       = byte_size / singile_bone_frame;
                if (byte_size % singile_bone_frame != 0) {
                    LOG_ERROR("wrong bone frame size %d", byte_size);
                    return false;
                }
                bframes.frames.resize(num);
                for (auto& frame : bframes.frames) {
                    for (auto& v : frame.position) v = f.ReadFloat();
                    for (auto& v : frame.angle) v = f.ReadFloat();
                    for (auto& v : frame.scale) v = f.ReadFloat();
                }
            }
            uint32_t unk_extra_uint = f.ReadUint32();
            for (uint i = 0; i < unk_extra_uint; i++) {
                f.ReadFloat();
                // data is like: {"$$hashKey":"object:2110","frame":1,"name":"random_anim"}
                std::string unk_extra = f.ReadStr();
            }
        }
    }
    mdl.puppet->prepared();

    LOG_INFO("----puppet----");
    LOG_INFO("mdlv: %d\nmdls: %d\nmdla: %d", mdl.mdlv, mdl.mdls, mdl.mdla);
    LOG_INFO("%s", mdl.mat_json_file.c_str());
    LOG_INFO("vertex: %d", mdl.vertexs.size());
    LOG_INFO("bones: %d", mdl.puppet->bones.size());
    LOG_INFO("anims: %d", mdl.puppet->anims.size());
    return true;
}

void WPMdlParser::GenPuppetMesh(SceneMesh& mesh, const WPMdl& mdl) {
    SceneVertexArray vertex({ { WE_IN_POSITION.data(), VertexType::FLOAT3 },
                              { WE_IN_BLENDINDICES.data(), VertexType::UINT4 },
                              { WE_IN_BLENDWEIGHTS.data(), VertexType::FLOAT4 },
                              { WE_IN_TEXCOORD.data(), VertexType::FLOAT2 } },
                            mdl.vertexs.size());

    std::array<float, 16> one_vert;
    auto                  to_one = [](const WPMdl::Vertex& in, decltype(one_vert)& out) {
        uint offset = 0;
        memcpy(out.data() + 4 * (offset++), in.position.data(), sizeof(in.position));
        memcpy(out.data() + 4 * (offset++), in.blend_indices.data(), sizeof(in.blend_indices));
        memcpy(out.data() + 4 * (offset++), in.weight.data(), sizeof(in.weight));
        memcpy(out.data() + 4 * (offset++), in.texcoord.data(), sizeof(in.texcoord));
    };
    for (uint i = 0; i < mdl.vertexs.size(); i++) {
        auto& v = mdl.vertexs[i];
        to_one(v, one_vert);
        vertex.SetVertexs(i, 1, one_vert.data());
    }
    std::vector<uint32_t> indices;
    size_t                u16_count = mdl.indices.size() * 3;
    indices.resize(u16_count / 2 + 1);
    memcpy(indices.data(), mdl.indices.data(), u16_count * sizeof(uint16_t));

    mesh.AddVertexArray(std::move(vertex));
    mesh.AddIndexArray(SceneIndexArray(indices));
}

void WPMdlParser::AddPuppetShaderInfo(WPShaderInfo& info, const WPMdl& mdl) {
    info.combos["SKINNING"]  = 1;
    info.combos["BONECOUNT"] = mdl.puppet->bones.size();
}

void WPMdlParser::AddPuppetMatInfo(wpscene::WPMaterial& mat, const WPMdl& mdl) {
    mat.combos["SKINNING"]  = 1;
    mat.combos["BONECOUNT"] = mdl.puppet->bones.size();
    mat.use_puppet          = true;
}