#include <cstdint>

namespace wallpaper
{
namespace fs
{

template<typename T>
constexpr T bswap(T source);

template<>
inline constexpr uint64_t bswap<uint64_t>(uint64_t source) {
    return 0 | ((source & uint64_t(0x00000000000000ffull)) << 56) |
           ((source & uint64_t(0x000000000000ff00ull)) << 40) |
           ((source & uint64_t(0x0000000000ff0000ull)) << 24) |
           ((source & uint64_t(0x00000000ff000000ull)) << 8) |
           ((source & uint64_t(0x000000ff00000000ull)) >> 8) |
           ((source & uint64_t(0x0000ff0000000000ull)) >> 24) |
           ((source & uint64_t(0x00ff000000000000ull)) >> 40) |
           ((source & uint64_t(0xff00000000000000ull)) >> 56);
}

template<>
inline constexpr uint32_t bswap<uint32_t>(uint32_t source) {
    return 0 | ((source & 0x000000ff) << 24) | ((source & 0x0000ff00) << 8) |
           ((source & 0x00ff0000) >> 8) | ((source & 0xff000000) >> 24);
}

template<>
inline constexpr uint16_t bswap<uint16_t>(uint16_t source) {
    return 0 | (uint16_t)((source & 0x00ff) << 8) | (uint16_t)((source & 0xff00) >> 8);
}

template<>
inline constexpr uint8_t bswap<uint8_t>(uint8_t source) {
    return source;
}

template<>
inline constexpr int64_t bswap<int64_t>(int64_t source) {
    return (int64_t)(bswap<uint64_t>((uint64_t)(source)));
}
template<>
inline constexpr int32_t bswap<int32_t>(int32_t source) {
    return (int32_t)(bswap<uint32_t>((uint32_t)(source)));
}
template<>
inline constexpr int16_t bswap<int16_t>(int16_t source) {
    return (int16_t)(bswap<uint16_t>((uint16_t)(source)));
}
template<>
inline constexpr int8_t bswap<int8_t>(int8_t source) {
    return (int8_t)(bswap<uint8_t>((uint8_t)(source)));
}
} // namespace fs
} // namespace wallpaper
