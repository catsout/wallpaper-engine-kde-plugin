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

template<typename T>
class spanone {
public:
    using value_type = T;
    using size_type  = size_t;
    using reference  = T&;
    using pointer    = T*;

    constexpr spanone(reference value) noexcept: ptr { &value } {}
    constexpr pointer   data() const noexcept { return ptr; }
    constexpr size_type size() const noexcept { return 1; }
    constexpr reference operator[](std::size_t index) const noexcept { return ptr[index]; }
    constexpr pointer   begin() const noexcept { return ptr; }
    constexpr pointer   end() const noexcept { return ptr + 1; }
    constexpr pointer   cbegin() const noexcept { return ptr; }
    constexpr pointer   cend() const noexcept { return ptr + 1; }

private:
    pointer ptr;
};

} // namespace wallpaper
