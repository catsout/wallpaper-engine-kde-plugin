#pragma once
#include <string_view>

namespace wallpaper {

constexpr std::string_view Tex_Default { "_rt_default" };
inline bool IsSpecTex(std::string_view name) {
	return name.compare(0, 4, "_rt_") == 0;
}
}