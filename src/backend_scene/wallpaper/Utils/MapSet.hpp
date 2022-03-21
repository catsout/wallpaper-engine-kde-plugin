#pragma once

#include <map>
#include <set>

namespace wallpaper
{

template<class Key, class Value>
using Map = std::map<Key, Value, std::less<>>;

template<class Key>
using Set = std::set<Key, std::less<>>;

template<class Key, class Value, class KeyLike, class Allocator>
inline bool exists(const std::map<Key, Value, std::less<>, Allocator>& m, const KeyLike& key) noexcept {
    auto iter = m.find(key);
    return iter != m.end();
}

template<class Key, class KeyLike, class Allocator>
inline bool exists(const std::set<Key, std::less<>, Allocator>& m, const KeyLike& key) noexcept {
    auto iter = m.find(key);
    return iter != m.end();
}

}