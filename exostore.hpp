#ifndef __EXOREDIS_EXOSTORE_HPP__
#define __EXOREDIS_EXOSTORE_HPP__

#include <string>
#include <vector>
#include <stdexcept>
#include "binary_string.hpp"

class exostore
{
public:
    typedef binary_string bstring;
    class key_error: public std::runtime_error
    {
    public:
        key_error() : runtime_error("Key does not exist") {}
    };

    class type_error: public std::runtime_error
    {
    public:
        type_error() : runtime_error("Value is of the wrong type") {}
    };

    exostore(std::string file_path);

    bool key_exists(const vector<unsigned char>& key);

    // Should throw if the key does not exist or if it expires.
    template <typename T>
    T& get(const vector<unsigned char>& key);

    template <typename T>
    void set(const vector<unsigned char>& key, const T& value);

    void expire_keys();
};

#endif
