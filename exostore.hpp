#ifndef __EXOREDIS_EXOSTORE_HPP__
#define __EXOREDIS_EXOSTORE_HPP__

#include <string>
#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <boost/unordered_map.hpp>
#include <boost/any.hpp>
#include "binary_string.hpp"
#include "sorted_set.hpp"

class exostore
{
public:
    typedef binary_string bstring;
    typedef sorted_set zset;

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

    // Should expire a key if needed.
    bool key_exists(const std::vector<unsigned char>& key);

    template <typename T>
    bool is_type(const std::vector<unsigned char>& key);

    // Should throw if the key does not exist or if it expires.
    template <typename T>
    T& get(const std::vector<unsigned char>& key);

    template <typename T>
    void set(const std::vector<unsigned char>& key, const T& value);

    void expire_keys();

    void save();

private:
    void load();
    // Returns true if the key was expired.
    bool expire_if_needed(const std::vector<unsigned char>& key);
    std::string db_path_;
    boost::unordered_map<std::vector<unsigned char>, boost::any> map_;
};

template <typename T>
bool exostore::is_type(const std::vector<unsigned char>& key)
{
    if (!key_exists(key))
    {
        return false;
    }
    return map_[key].type() == typeid(T);
}

template <typename T>
T& exostore::get(const std::vector<unsigned char>& key)
{
    if (map_.count(key) == 0 || expire_if_needed(key))
    {
        throw exostore::key_error();
    }

    auto& value = map_.at(key);
    if (value.type() != typeid(T))
    {
        throw exostore::type_error();
    }

    try
    {
        return boost::any_cast<T&>(value);
    }
    catch (boost::bad_any_cast& )   // This shouldn't happen
    {
        throw exostore::type_error();
    }
}

template <typename T>
void exostore::set(const std::vector<unsigned char>& key, const T& value)
{
    map_[key] = value;
}

#endif
