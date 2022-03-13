#include "WPMdlParser.hpp"
#include "Fs/IBinaryStream.h"
#include "WPCommon.hpp"
#include "Utils/Logging.h"

using namespace wallpaper;

// bytes * size
constexpr uint32_t singile_vertex = 4*(3+4+4+2);
constexpr uint32_t singile_indices = 2*3;

bool WPMdlParser::Parse(fs::IBinaryStream& f, WPMdl& mdl) {
    mdl.mdlv = ReadMDLVesion(f);
    int32_t num_uknown = f.ReadInt32();
    int32_t num1 = f.ReadInt32();
    int32_t num2 = f.ReadInt32();
    mdl.mat_json_file = f.ReadStr();
    // 0
    f.ReadInt32();

    uint32_t vertex_size = f.ReadUint32();
    if(vertex_size % singile_vertex != 0)  {
        LOG_ERROR("unsupport mdl vertex size %d", vertex_size);
        return false;
    }

    uint32_t vertex_num = vertex_size / singile_vertex;
    mdl.vertexs.resize(vertex_num);
    for(auto& vert:mdl.vertexs) {
        for(auto& v:vert.position) v = f.ReadFloat();
        for(auto& v:vert.blend_indices) v = f.ReadUint32();
        for(auto& v:vert.weight) v = f.ReadFloat();
        for(auto& v:vert.texcoord) v = f.ReadFloat();
    }

    uint32_t indices_size = f.ReadUint32();
    if(indices_size % singile_indices != 0) {
        LOG_ERROR("unsupport mdl indices size %d", indices_size);
        return false;
    }

    uint32_t indices_num = indices_size / singile_indices;
    mdl.indices.resize(indices_num);
    for(auto& id:mdl.indices) {
        for(auto& v:id) v = f.ReadUint16();
    }

    mdl.mdls = ReadMDLVesion(f);
    // unknown
    f.ReadInt32();

    uint16_t bones_num = f.ReadUint16();
    // 1 byte
    {char c;f.Read(&c, 1);}

    // uint16 unk
    // int32 
    // int32 
    // int32 size
    // mat4x3 bnoes
    // vec4 unk_pos
    mdl.bones.resize(bones_num);
    for(auto& bone:mdl.bones) {
        int32_t unk1 = f.ReadUint16();
        int32_t unk2 = f.ReadInt32();
        int32_t unk3 = f.ReadInt32();

        uint32_t size = f.ReadUint32();
        if(size != 64) {
            LOG_ERROR("mdl unsupport bones size: %d", size);
            return false;
        }
        for(auto& b:bone) b = f.ReadFloat();
        std::array<float, 4> unk_pos;
        for(auto& v:unk_pos) v = f.ReadFloat();
    }
    // 1 byte
    {char c; f.Read(&c, 1);}

    mdl.mdla = ReadMDLVesion(f);
    LOG_INFO("----puppet----");
    LOG_INFO("mdlv: %d\nmdls: %d\nmdla: %d", mdl.mdlv, mdl.mdls, mdl.mdla);
    LOG_INFO("%s", mdl.mat_json_file.c_str());
    LOG_INFO("vertex: %d", mdl.vertexs.size());
    LOG_INFO("bones: %d", mdl.bones.size());
    return true;
}