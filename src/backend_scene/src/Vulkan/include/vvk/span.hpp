#pragma once

#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>
#include <iterator>

namespace vvk
{

// vulkan: size_type -> u32
template<typename T>
class Span {
public:
    using value_type             = T;
    using u32                    = uint32_t;
    using size_type              = std::size_t;
    using size_type_out          = u32;
    using difference_type        = std::ptrdiff_t;
    using reference              = T&;
    using const_reference        = const T&;
    using nonconst_reference     = std::remove_const_t<T>&;
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

    /// Define non-const reference construct for const value_type
    template<typename U = value_type, typename = std::enable_if_t<std::is_const_v<U>>>
    constexpr Span(nonconst_reference value) noexcept: ptr { &value }, num { 1 } {}

    /// Construct a span from a range.
    // requires std::data(Range&), std::size(Range&)
    template<typename Range>
    constexpr Span(const Range& range) noexcept
        : ptr { std::data(range) }, num { std::size(range) } {}

    template<typename Range>
    constexpr Span(Range& range) noexcept: ptr { std::data(range) }, num { std::size(range) } {}

    /// Construct a span from a pointer and a size.
    /// This is inteded for subranges.
    constexpr Span(pointer ptr_, size_type num_) noexcept: ptr { ptr_ }, num { num_ } {}

    constexpr pointer data() const noexcept { return ptr; }

    constexpr size_type_out size() const noexcept { return static_cast<size_type_out>(num); }

    constexpr bool empty() const noexcept { return num == 0; }

    /// @pre: index < size()
    constexpr reference operator[](std::size_t index) const noexcept { return ptr[index]; }

    constexpr pointer begin() const noexcept { return ptr; }
    constexpr pointer end() const noexcept { return ptr + num; }
    constexpr pointer cbegin() const noexcept { return ptr; }
    constexpr pointer cend() const noexcept { return ptr + num; }
    /*
        constexpr pointer begin() noexcept { return ptr; }
        constexpr pointer end() noexcept { return ptr + num; }
        constexpr pointer cbegin() noexcept { return ptr; }
        constexpr pointer cend() noexcept { return ptr + num; }
    */

private:
    pointer   ptr { nullptr };
    size_type num { 0 };
};

template<typename T, typename R>
constexpr bool operator==(const Span<T>& lhs, const Span<R>& rhs) noexcept {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
} // namespace vvk
