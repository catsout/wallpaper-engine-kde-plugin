#pragma once
#include <array>
#include <vector>
#include <span>
#include <algorithm>

namespace wallpaper
{

template<typename T, typename Tarray>
std::array<T, std::tuple_size<Tarray>::value> array_cast(const Tarray& array) noexcept {
    std::array<T, std::tuple_size<Tarray>::value> res;
    std::copy(array.begin(), array.end(), res.begin());
    return res;
}

template<typename S, typename TFunc, typename TR = typename std::result_of<TFunc(S)>::type>
std::vector<TR> transform(std::span<const S> src, TFunc&& func) {
    std::vector<TR> dst(std::size(src));
    std::transform(std::begin(src), std::end(src), std::begin(dst), func);
    return dst;
}

} // namespace wallpaper
