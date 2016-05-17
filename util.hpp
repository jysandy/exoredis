#ifndef __EXOREDIS_UTIL_HPP__
#define __EXOREDIS_UTIL_HPP__

#include <vector>
#include <string>
#include <locale>

std::string toupper_string(const std::string& input)
{
    std::locale loc;
    std::string output;
    for (auto c: input)
    {
        output += std::toupper(c, loc);
    }
    return output;
}

std::string vec_to_string(const vector<unsigned char>& v)
{
    return std::string(v.begin(), v.end());
}

// Necessary for tokenizing a vector<unsigned char>.
void operator+=(std::vector<unsigned char>& v, unsigned char c)
{
    v.push_back(c);
}

#endif
