#ifndef __EXOREDIS_BINARY_STRING_HPP__
#define __EXOREDIS_BINARY_STRING_HPP__

#include <vector>
#include <chrono>

namespace chrono = std::chrono;

class binary_string
{
public:
    binary_string();
    binary_string(const std::vector<unsigned char>& bdata);    // No expiry time for this one
    binary_string(const std::vector<unsigned char>& bdata,
        long long expiry_milliseconds);

    const std::vector<unsigned char>& bdata() const;
    std::vector<unsigned char>& bdata();
    bool has_expired() const;
private:
    bool expiry_set_;
    chrono::system_clock::time_point expiry_time_;
    std::vector<unsigned char> bdata_;
};

#endif
