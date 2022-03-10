#include "Sha.hpp"
#include <vog/sha1.hpp>
using namespace wallpaper::utils;

std::string wallpaper::utils::genSha1(Span<char> in) {
    SHA1 sha1;
    sha1.update(std::string(in.data(), in.size()));
    return sha1.final();
}