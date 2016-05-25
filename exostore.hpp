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


/*
 * The fundamental database type. Responsible for managing the database in the
 * form of a hash table. Exposes functions to get and set data, and to expire
 * keys if needed.
 */
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

    class load_error: public std::runtime_error
    {
    public:
        load_error() : runtime_error("Loading from file failed") {}
        load_error(std::string msg) : runtime_error(msg) {}
    };

    exostore(std::string file_path);

    // Should expire a key if needed.
    bool key_exists(const std::vector<unsigned char>& key);

    // Checks if a key is of type T
    template <typename T>
    bool is_type(const std::vector<unsigned char>& key);

    // Gets the value of type T stored at key.
    // Should throw if the key does not exist or if it expires.
    // Should throw if the value is the wrong type.
    template <typename T>
    T& get(const std::vector<unsigned char>& key);

    // Sets the given key to the given value.
    template <typename T>
    void set(const std::vector<unsigned char>& key, const T& value);

    // Removes all expired keys from the hash table/
    void expire_keys();

    // Save to disk.
    void save();
    // Load from disk.
    void load();

private:
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
