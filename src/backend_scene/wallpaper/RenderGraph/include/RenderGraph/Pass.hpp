#pragma once
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{
namespace rg
{

class Pass : NoCopy {

};

class VirtualPass : public Pass {
public:
    struct Desc {};
    VirtualPass(const Desc&) noexcept {}; 
    virtual ~VirtualPass() noexcept {}; 
};
}
}
