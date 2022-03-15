#pragma once
#include <string_view>

namespace wallpaper
{

constexpr bool sstart_with(std::string_view str, std::string_view start) {
    return str.size() >= start.size() &&
        str.compare(0, start.size(), start, 0, start.size()) == 0;
} 

}