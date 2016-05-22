#include "exostore.hpp"

#include <typeinfo>

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

void exostore::save()
{
    // TODO: Implement
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
