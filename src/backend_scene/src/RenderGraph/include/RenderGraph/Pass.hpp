#pragma once
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{
namespace rg
{

class Pass : NoCopy {
public:
    Pass()          = default;
    virtual ~Pass() = default;
};

class VirtualPass : public Pass {
public:
    struct Desc {};
    VirtualPass(const Desc&) noexcept {};
    virtual ~VirtualPass() noexcept {};
};
} // namespace rg
} // namespace wallpaper
