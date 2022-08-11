#pragma once
#include <string>
#include <span>

namespace utils
{
constexpr size_t SHA1_LEN = 40;

std::string genSha1(std::span<const char>);
} // namespace utils
