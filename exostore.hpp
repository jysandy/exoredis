#ifndef __EXOREDIS_EXOSTORE_HPP__
#define __EXOREDIS_EXOSTORE_HPP__

#include <string>
#include <vector>

class exostore
{
public:
    typedef vector<unsigned char> bstring;

    exostore(std::string file_path);

    bool key_exists(const vector<unsigned char>& key);

    template <typename T>
    T& get(const vector<unsigned char>& key);

    template <typename T>
    void set(const vector<unsigned char>& key, const T& value);
};

#endif
