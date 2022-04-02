#pragma once
#include <string_view>
#include "Fs/IBinaryStream.h"
#include "Utils/Logging.h"

namespace wallpaper
{

inline int32_t ReadVesion(std::string_view prefix, fs::IBinaryStream& file) {
    char str_v[9];
    file.Read(str_v, 9);
    if (prefix.compare(0, 3, str_v, 3) != 0) return 0;

    //	std::cout << str_v << std::endl;
    return std::stoi(str_v + 4);
}
inline int32_t ReadTexVesion(fs::IBinaryStream& file) { return ReadVesion("TEX", file); }
inline int32_t ReadMDLVesion(fs::IBinaryStream& file) { return ReadVesion("MDL", file); }

} // namespace wallpaper