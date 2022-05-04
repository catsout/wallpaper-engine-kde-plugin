#pragma once
#include <bitset>

namespace wallpaper
{

template<typename EnumT>
class BitFlags {
    static_assert(std::is_enum_v<EnumT>, "Flags can only be specialized for enum types");

    using UnderlyingT = typename std::make_unsigned_t<typename std::underlying_type_t<EnumT>>;

public:
    constexpr BitFlags() noexcept: bits_(0u) {}
    constexpr BitFlags(UnderlyingT val) noexcept: bits_(val) {}

    BitFlags& set(EnumT e, bool value = true) noexcept {
        bits_.set(underlying(e), value);
        return *this;
    }

    BitFlags& reset(EnumT e) noexcept {
        set(e, false);
        return *this;
    }

    BitFlags& reset() noexcept {
        bits_.reset();
        return *this;
    }

    [[nodiscard]] bool all() const noexcept { return bits_.all(); }

    [[nodiscard]] bool any() const noexcept { return bits_.any(); }

    [[nodiscard]] bool none() const noexcept { return bits_.none(); }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return bits_.size(); }

    [[nodiscard]] std::size_t count() const noexcept { return bits_.count(); }

    constexpr bool operator[](EnumT e) const { return bits_[underlying(e)]; }

    constexpr bool operator[](UnderlyingT t) const { return bits_[t]; }

    auto to_string() const { return bits_.to_string(); }

private:
    static constexpr UnderlyingT underlying(EnumT e) { return static_cast<UnderlyingT>(e); }

private:
    std::bitset<sizeof(UnderlyingT) * 8> bits_;
};
} // namespace wallpaper