#ifndef __EXOREDIS_BINARY_STRING_HPP__
#define __EXOREDIS_BINARY_STRING_HPP__

#include <vector>
#include <chrono>

namespace chrono = std::chrono;

class binary_string
{
public:
    binary_string();
    binary_string(std::vector<unsigned char> bdata);    // No expiry time for this one
    binary_string(std::vector<unsigned char> bdata, long long expiry_milliseconds);

    const std::vector<unsigned char>& bdata() const;

};

#endif
