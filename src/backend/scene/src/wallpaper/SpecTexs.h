#pragma once
#include <string_view>

namespace wallpaper {

constexpr std::string_view SpecTex_Default { "_rt_default" };

inline bool IsSpecTex(const std::string_view name) {
	return name.compare(0, 4, "_rt_") == 0;
}
}