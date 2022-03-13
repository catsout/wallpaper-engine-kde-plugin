#pragma once
#include <string>
#include <cstdint>
#include <array>
#include <vector>

namespace wallpaper 
{

namespace fs { class IBinaryStream; }

struct WPMdl {
    uint mdlv {13};
    uint mdls {1};
    uint mdla {1};

    std::string mat_json_file;
    struct Vertex {
        std::array<float,3> position;
        std::array<uint32_t,4> blend_indices;
        std::array<float,4> weight;
        std::array<float,2> texcoord;
    };
    std::vector<Vertex> vertexs;
    std::vector<std::array<uint16_t,3>> indices;

    std::vector<std::array<float,3*4>> bones;
    // combo
    // SKINNING = 1
    // BONECOUNT

    // input
    // uvec4 a_BlendIndices
    // vec4 a_BlendWeights
    // uniform mat4x3 g_Bones[BONECOUNT]
};

class WPMdlParser {
public:
	static bool Parse(fs::IBinaryStream&, WPMdl&);
};

}

