#include "Sha.hpp"
#include <vog/sha1.hpp>

std::string utils::genSha1(Span<const char> in) {
    SHA1 sha1;
    sha1.update(std::string(in.data(), in.size()));
    return sha1.final();
}
