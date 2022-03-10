#pragma once
#include <array>

namespace wallpaper
{

template <typename T, typename Tarray>
std::array<T, std::tuple_size<Tarray>::value> array_cast(const Tarray& array) noexcept {
    std::array<T, std::tuple_size<Tarray>::value> res;
    std::copy(array.begin(), array.end(), res.begin());
    return res;
}

}