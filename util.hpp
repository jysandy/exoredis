#ifndef __EXOREDIS_UTIL_HPP__
#define __EXOREDIS_UTIL_HPP__

#include <vector>
#include <string>
#include <locale>

std::string toupper_string(const std::string& input);

std::vector<unsigned char> string_to_vec(const std::string& s);

std::string vec_to_string(const std::vector<unsigned char>& v);

// Necessary for tokenizing a vector<unsigned char>.
void operator+=(std::vector<unsigned char>& v, unsigned char c);

#endif
