#pragma once
#include <string_view>
#include <charconv>
#include "Fs/IBinaryStream.h"
#include "Utils/Logging.h"
#include "Core/StringHelper.hpp"

namespace wallpaper
{

inline int32_t ReadVersion(std::string_view prefix, fs::IBinaryStream& file) {
    char str_v[9] { '\0' };
    file.Read(str_v, 9);
    if (! sstart_with(str_v, prefix)) return 0;

    char* str_int = str_v + 4;
    int   slot;
    auto [ptr, ec] { std::from_chars(str_int, std::end(str_v), slot) };
    if (ec != std::errc()) {
        LOG_ERROR("read version of \'%.*s\' failed", 8, str_v);
        return 0;
    }
    return slot;
}
inline void WriteVersion(std::string_view prefix, fs::IBinaryStreamW& file, int ver) {
    char buf[9] { '\0' };
    std::snprintf(buf, sizeof(buf), "%.4s%.4d", prefix.data(), ver);
    file.Write(buf, sizeof(buf));
}

inline int32_t ReadTexVesion(fs::IBinaryStream& file) { return ReadVersion("TEX", file); }
inline int32_t ReadMDLVesion(fs::IBinaryStream& file) { return ReadVersion("MDL", file); }

// DIY
inline int32_t ReadSPVVesion(fs::IBinaryStream& file) { return ReadVersion("SPV", file); }
inline void WriteSPVVesion(fs::IBinaryStreamW& file, int ver) { WriteVersion("SPVS", file, ver); }

} // namespace wallpaper
