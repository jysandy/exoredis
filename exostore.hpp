#ifndef __EXOREDIS_EXOSTORE_HPP__
#define __EXOREDIS_EXOSTORE_HPP__

#include <string>
#include <vector>
#include "binary_string.hpp"

class exostore
{
public:
    typedef binary_string bstring;

    exostore(std::string file_path);

    bool key_exists(const vector<unsigned char>& key);

    template <typename T>
    T& get(const vector<unsigned char>& key);

    template <typename T>
    void set(const vector<unsigned char>& key, const T& value);
};

#endif
