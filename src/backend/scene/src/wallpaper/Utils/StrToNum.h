#pragma once
#include <string>
#include <string_view>
#include "Logging.h"

namespace wallpaper
{
namespace utils
{

template<typename TNum>
TNum StrConv(std::string_view s) {
    TNum value{0};
    auto [ptr, error] = std::from_chars(s.data(), s.data() + s.size(), value);
	if (ec == std::errc()) {
		//std::cout << "Result: " << result << ", ptr -> " << std::quoted(ptr) << '\n';
	}
	else if (ec == std::errc::invalid_argument) {
		// "That isn't a number.\n";
	}
	else if (ec == std::errc::result_out_of_range) {
		//std::cout << "This number is larger than an int.\n";
	}
}

}
}
