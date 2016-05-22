#include "exostore.hpp"
#include "util.hpp"

#include <typeinfo>
#include <fstream>
#include <cstddef>
#include <utility>

exostore::exostore(std::string file_path)
    : db_path_(file_path)
{
    load();
}

bool exostore::key_exists(const std::vector<unsigned char>& key)
{
    return map_.count(key) != 0 && !expire_if_needed(key);
}



void exostore::expire_keys()
{
    for (auto it = map_.begin(); it != map_.end(); )
    {
        if (it->second.type() == typeid(exostore::bstring))
        {
            auto& val = boost::any_cast<exostore::bstring&>(it->second);
            if (val.has_expired())
            {
                it = map_.erase(it);
                continue;
            }
        }
        it++;
    }
}

/*
 *  The file starts with the bytes EXODB.
 *  Then the number of key-value pairs which is an std::size_t.
 *  Then each key value pair. The key is preceded by the number of bytes,
 *  which is also a size_t.
 *  The value is preceded by either BSTR or ZSET, indicating the type of the
 *  value.
 *  A BSTR is represented as its size in bytes (size_t) followed by the contents.
 *  A ZSET is represented by the number of members, followed by score-member pairs
 *  in no defined order.
 *  A score-member pair is a 64-bit double, followed by the size of the member
 *  in bytes, then the contents of the member.
 */

void exostore::save()
{
    // Expire all keys first.
    expire_keys();

    std::ofstream out(db_path_, std::ofstream::out|std::ofstream::binary);
    auto header = string_to_vec("EXODB");
    out.write(reinterpret_cast<const char*>(header.data()), header.size());
    
}

void exostore::load()
{
    // TODO: Implement
}

bool exostore::expire_if_needed(const std::vector<unsigned char>& key)
{
    if (map_.count(key) == 0)
    {
        return false;
    }

    auto& value = map_.at(key);
    if (value.type() != typeid(exostore::bstring))
    {
        return false;
    }

    auto& bstr = boost::any_cast<exostore::bstring&>(value);
    if (bstr.has_expired())
    {
        map_.erase(key);
        return true;
    }
    else
    {
        return false;
    }
}
