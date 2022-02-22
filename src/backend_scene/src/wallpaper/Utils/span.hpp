#pragma once

#include <cstdint>
#include <algorithm>

template <typename T>
class Span {
public:
    using value_type = T;
    using u32 = uint32_t;
    using size_type = u32;
    using difference_type = std::ptrdiff_t;
    using reference = const T&;
    using const_reference = const T&;
    using pointer = const T*;
    using const_pointer = const T*;
    using iterator = const T*;
    using const_iterator = const T*;

    /// Construct an empty span.
    constexpr Span() noexcept = default;

    /// Construct an empty span
    constexpr Span(std::nullptr_t) noexcept {}

    /// Construct a span from a single element.
    constexpr Span(const T& value) noexcept : ptr{&value}, num{1} {}

    /// Construct a span from a range.
    template <typename Range>
    // requires std::data(const Range&)
    // requires std::size(const Range&)
    constexpr Span(const Range& range) : ptr{std::data(range)}, num{std::size(range)} {}

    /// Construct a span from a pointer and a size.
    /// This is inteded for subranges.
    constexpr Span(const T* ptr_, std::size_t num_) noexcept : ptr{ptr_}, num{num_} {}

    constexpr const T* data() const noexcept {
        return ptr;
    }

    constexpr u32 size() const noexcept {
        return static_cast<u32>(num);
    }

    constexpr bool empty() const noexcept {
        return num == 0;
    }

    /// @pre: index < size()
    constexpr const T& operator[](std::size_t index) const noexcept {
        return ptr[index];
    }

    constexpr const T* begin() const noexcept {
        return ptr;
    }

    constexpr const T* end() const noexcept {
        return ptr + num;
    }

    constexpr const T* cbegin() const noexcept {
        return ptr;
    }

    constexpr const T* cend() const noexcept {
        return ptr + num;
    }

private:
    const T* ptr = nullptr;
    std::size_t num = 0;
};

template<typename T, typename R>
constexpr bool operator==(const Span<T>& lhs, const Span<R>& rhs) noexcept {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}