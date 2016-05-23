#include "binary_string.hpp"

binary_string::binary_string()
    : expiry_set_(false)
{
}

binary_string::binary_string(const std::vector<unsigned char>& bdata)
    : expiry_set_(false), bdata_(bdata)
{
}

binary_string::binary_string(const std::vector<unsigned char>& bdata,
    long long expiry_milliseconds)
    : bdata_(bdata), expiry_set_(true)
{
    expiry_time_ = chrono::system_clock::now()
        + chrono::milliseconds(expiry_milliseconds);
}

const std::vector<unsigned char>& binary_string::bdata() const
{
    return bdata_;
}

std::vector<unsigned char>& binary_string::bdata()
{
    return bdata_;
}

bool binary_string::has_expired() const
{
    if (!expiry_set_)
    {
        return false;
    }

    return expiry_time_ < chrono::system_clock::now();
}
