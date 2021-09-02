#pragma once
#include <string_view>
#include <cstdint>
#include "Util.h"

namespace wallpaper {

constexpr std::string_view SpecTex_Default { "_rt_default" };
constexpr std::string_view SpecTex_Link { "_rt_link_" };

inline bool IsSpecTex(const std::string_view name) {
	return name.compare(0, 4, "_rt_") == 0;
}
inline bool IsSpecLinkTex(const std::string_view name) {
	return name.find(SpecTex_Link) != std::string_view::npos;
}
inline uint32_t ParseLinkTex(const std::string_view name) {
	std::string sid {name};
	sid = sid.substr(9);
	return StrConv<uint32_t>(sid);
}
inline std::string GenLinkTex(uint32_t id) {
	return std::string(SpecTex_Link) + std::to_string(id);
}

}