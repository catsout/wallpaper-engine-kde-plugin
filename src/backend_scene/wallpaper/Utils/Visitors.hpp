#pragma once

namespace wallpaper 
{
namespace visitor
{

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

struct NoType {};

struct EqualVisitor
{
    using result_type = bool;

    template <typename T, typename U>
    bool operator()(const T&, const U&) const
    {
        // Different types : not equal
        return false;
    }

    template <typename T>
    bool operator()(const T& v1, const T& v2) const
    {
        // Same types : compare values
        return v1 == v2;
    }

    bool operator()(NoType, NoType) const
    {
        // No type (empty values) : they're considered equal
        return true;
    }
};

}
}