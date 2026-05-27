#pragma once

#include <cstdint>
#include <cstring>
#include <string_view>

namespace mb {

typedef std::string File_Hash;

// Calculate hash for a file - returns {hash, success}
std::tuple<File_Hash, bool> file_hash(std::string_view _file_name);

// Check file's hash looking for _file_name + ".hash" and use the file_hash()
bool check_file_hash(std::string_view _file_name);

// Create a file _file_name + ".hash" and use the file_hash()
void create_file_hash(std::string_view _file_name);

// FNV-1a 32bit hashing algorithm.
constexpr std::uint32_t strhash(char const *s, std::size_t count)
{
    return ((count ? strhash(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

constexpr std::uint32_t strhash(char const *s)
{
    return strhash(s, strlen(s));
}

// FNV-1a 32bit hashing algorithm. - Forcing ASCII lower letters
constexpr std::uint32_t l_strhash(char const *s, std::size_t count)
{
    return ((count ? l_strhash(s, count - 1) : 2166136261u) ^ (s[count] | 0b100000)) * 16777619u;
}

constexpr std::uint32_t l_strhash(const char *s)
{
    return l_strhash(s, strlen(s));
}

template<typename T>
inline std::uint32_t strhash(const T &s)
{
    return strhash(s.data(), s.size());
}

template<typename T>
inline std::uint32_t l_strhash(const T &s)
{
    return l_strhash(s.data(), s.size());
}

constexpr std::uint32_t operator"" _hash(char const *s, std::size_t count)
{
    return strhash(s, count);
}

constexpr std::uint32_t operator"" _lhash(char const *s, std::size_t count)
{
    return l_strhash(s, count);
}

} // namespace mb
