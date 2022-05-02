#pragma once

#include <cstdint>
#include <algorithm>
#include <vector>
#include <iterator>

template<typename T>
class Span {
public:
    using value_type             = T;
    using u32                    = uint32_t;
    using size_type              = size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = T&;
    using const_reference        = const reference;
    using pointer                = T*;
    using const_pointer          = const pointer;
    using iterator               = T*;
    using const_iterator         = const iterator;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /// Construct an empty span.
    constexpr Span() noexcept = default;

    /// Construct an empty span
    constexpr Span(std::nullptr_t) noexcept {}

    /// Construct a span from a single element.
    constexpr Span(reference value) noexcept: ptr { &value }, num { 1 } {}

    /// Construct a span from a range.
    template<typename Range>
    // requires std::data(const Range&)
    // requires std::size(const Range&)
    constexpr Span(const Range& range): ptr { std::data(range) }, num { std::size(range) } {}

    /// Construct a span from a pointer and a size.
    /// This is inteded for subranges.
    constexpr Span(pointer ptr_, size_type num_) noexcept: ptr { ptr_ }, num { num_ } {}

    constexpr const_pointer data() const noexcept { return ptr; }

    constexpr pointer data() noexcept { return ptr; }

    constexpr size_type size() const noexcept { return num; }

    constexpr bool empty() const noexcept { return num == 0; }

    /// @pre: index < size()
    constexpr const_reference operator[](std::size_t index) const noexcept { return ptr[index]; }

    constexpr const_pointer begin() const noexcept { return ptr; }
    constexpr const_pointer end() const noexcept { return ptr + num; }
    constexpr const_pointer cbegin() const noexcept { return ptr; }
    constexpr const_pointer cend() const noexcept { return ptr + num; }

    constexpr pointer begin() noexcept { return ptr; }
    constexpr pointer end() noexcept { return ptr + num; }
    constexpr pointer cbegin() noexcept { return ptr; }
    constexpr pointer cend() noexcept { return ptr + num; }

private:
    pointer   ptr { nullptr };
    size_type num { 0 };
};

template<typename T, typename R>
constexpr bool operator==(const Span<T>& lhs, const Span<R>& rhs) noexcept {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
