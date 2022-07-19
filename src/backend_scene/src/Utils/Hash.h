#include <functional>

namespace utils 
{

// from boost (functional/hash):
// see http://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html template
template <typename T>
inline void hash_combine(std::size_t &seed, const T &val) {
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T>
inline void hash_combine_fast(std::size_t &seed, const T &val) {
    seed ^= std::hash<T>()(val) << 1u;
}

}


