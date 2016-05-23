#include "util.hpp"

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

std::vector<unsigned char> string_to_vec(const std::string& s)
{
    return std::vector<unsigned char>(s.begin(), s.end());
}

std::string vec_to_string(const std::vector<unsigned char>& v)
{
    return std::string(v.begin(), v.end());
}

// Necessary for tokenizing a vector<unsigned char>.
void operator+=(std::vector<unsigned char>& v, unsigned char c)
{
    v.push_back(c);
}

std::vector<unsigned char> vec_from_file(std::ifstream& in, std::size_t bytes)
{
    char c;
    std::vector<unsigned char> ret;
    for (std::size_t i = 0; i < bytes && in; i++)
    {
        in.get(c);
        ret.push_back(static_cast<unsigned char>(c));
    }
    return std::move(ret);
}
