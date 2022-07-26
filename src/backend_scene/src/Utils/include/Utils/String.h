#pragma once
#include <algorithm>
#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>
#include "Logging.h"
#include "Identity.hpp"

#define STRTONUM(s, num) utils::StrToNum(s, num, __SHORT_FILE__, __LINE__);

namespace utils
{

template<typename TNum>
void _StrToNum(std::string_view, TNum&) {}
template<>
inline void _StrToNum<int32_t>(std::string_view s, int32_t& num) {
    num = std::stoi(std::string(s));
}
template<>
inline void _StrToNum<uint32_t>(std::string_view s, uint32_t& num) {
    num = (uint32_t)std::stoul(std::string(s));
}
template<>
inline void _StrToNum<float>(std::string_view s, float& num) {
    num = std::stof(std::string(s));
}

template<typename TNum>
void StrToNum(std::string_view s, TNum& num, const char* file, int line) {
    try {
        _StrToNum(s, num);
    } catch (const std::invalid_argument& e) {
        WallpaperLog(LOGLEVEL_ERROR, file, line, "not a number");
    } catch (const std::out_of_range& e) {
        WallpaperLog(LOGLEVEL_ERROR, file, line, "too larger number");
    }
}

inline std::vector<std::string> SpliteString(std::string str, char spliter) {
    std::vector<std::string> result;
    std::size_t              pos = 0;
    while ((pos = str.find_first_of(spliter)) != std::string::npos) {
        result.push_back(str.substr(0, pos));
        str = str.substr(pos + 1);
    }
    result.push_back(str);
    return result;
}

namespace StrToArray
{
struct WrongSizeExp : public std::exception {
    const char* what() const throw() { return "Wrong size of the array"; }
};

template<typename T>
bool Convert(const std::string& str, std::vector<T>& target) {
    std::vector<std::string> str_list = SpliteString(str, ' ');
    if (target.size() < str_list.size()) target.resize(str_list.size());
    auto StrConv = [](std::string str) {
        T num {};
        _StrToNum(str, num);
        return num;
    };
    std::transform(str_list.begin(), str_list.end(), target.begin(), StrConv);
    return true;
}
template<typename T, std::size_t N>
bool Convert(const std::string& str, std::array<T, N>& target) {
    std::vector<std::string> str_list = SpliteString(str, ' ');
    if (N != str_list.size()) {
        throw WrongSizeExp();
    }
    auto StrConv = [](std::string str) {
        T num {};
        _StrToNum(str, num);
        return num;
    };
    std::transform(str_list.begin(), str_list.end(), target.begin(), StrConv);
    return true;
}

} // namespace StrToArray
} // namespace utils
