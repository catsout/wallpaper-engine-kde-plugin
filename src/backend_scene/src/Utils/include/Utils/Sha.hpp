#pragma once
#include <string>
#include "span.hpp"

namespace utils
{
constexpr size_t SHA1_LEN = 40;

std::string genSha1(Span<const char>);
} // namespace utils
