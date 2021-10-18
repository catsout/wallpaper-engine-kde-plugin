#pragma once
#include <vector>
#include <array>

namespace wallpaper {
namespace utils {

	template <typename>
	struct is_std_array { };
	// partial
	template <typename T>
	struct is_std_array<std::vector<T>> { using type = std::vector<T>; };
	template <typename T, std::size_t N>
	struct is_std_array<std::array<T, N>> { using type = std::array<T, N>; };
}
}
