#include "exostore.hpp"
#include "util.hpp"

#include <typeinfo>
#include <fstream>
#include <iostream>
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
    const auto bstr_marker = string_to_vec("BSTR");
    const auto zset_marker = string_to_vec("ZSET");
    // Write out the number of keys.
    std::size_t num_keys =  map_.size();
    out.write(reinterpret_cast<const char*>(&num_keys), sizeof(num_keys));
    for (const auto& pair: map_)
    {
        std::size_t key_size = pair.first.size();
        out.write(reinterpret_cast<const char*>(&key_size), sizeof(key_size));
        out.write(reinterpret_cast<const char*>(pair.first.data(),
            pair.first.size()));
        if (pair.second.type() == typeid(exostore::bstring))
        {
            // Write marker.
            out.write(reinterpret_cast<const char*>(bstr_marker.data()),
                bstr_marker.size());
            auto& bstring = boost::any_cast<exostore::bstring&>(pair.second);
            // Write bstring length.
            std::size_t bstring_size = bstring.bdata().size();
            out.write(reinterpret_cast<const char*>(&bstring_size),
                sizeof(bstring_size));
            // Write out the bstring.
            out.write(reinterpret_cast<const char*>(bstring.bdata().data()),
                bstring.bdata().size());
        }
        else if (pair.second.type() == typeid(exostore::zset))
        {
            // Write marker.
            out.write(reinterpret_cast<const char*>(zset_marker.data()),
                bstr_marker.size());
            auto& zset = boost::any_cast<exostore::zset&>(pair.second);
            // Write size of zset.
            std::size_t zset_size = zset.size();
            out.write(reinterpret_cast<const char*>(&zset_size),
                sizeof(zset_size));
            // Write out score-member pairs.
            auto its =  zset.element_range(0, zset_size - 1);
            for (auto it = its.first; it != its.second; it++)
            {
                // Write out score.
                auto score = it->score();
                out.write(reinterpret_cast<const char*>(&score), sizeof(score));
                // Write member size.
                std::size_t member_size = it->member.size();
                out.write(reinterpret_cast<const char*>(&member_size),
                    sizeof(member_size));
                // Write member contents.
                out.write(reinterpret_cast<const char*>(it->member.data()),
                    it->member.size());
            }
        }
    }
}

void exostore::load()
{

    std::ifstream in(db_path_, std::ifstream::binary);
    if (!in.is_open())
    {
        return;
    }
    in.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    // Add the keys to this map first, then copy only if there are no errors.
    boost::unordered_map<std::vector<unsigned char>, boost::any> temp_map_;
    try
    {
        // Read header.
        std::vector<unsigned char> header = vec_from_file(in, 5);
        if (vec_to_string(header) != "EXODB")
        {
            throw exostore::load_error("Bad file format");
        }


        // Read number of keys.
        std::size_t num_keys;
        in.read(reinterpret_cast<char*>(&num_keys), sizeof(num_keys));

        for (std::size_t i = 0; i < num_keys; i++)
        {
            // Read the key size.
            std::size_t key_size;
            in.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
            // Read the key.
            auto key = vec_from_file(in, key_size);
            // Read the value marker.
            auto value_marker = vec_from_file(in, 4);
            std::string marker_str = vec_to_string(value_marker);
            if (marker_str == "BSTR")   // Binary string
            {
                // Read bstring length.
                std::size_t bstring_size;
                in.read(reinterpret_cast<char*>(&bstring_size),
                    sizeof(bstring_size));
                // Read bstring content.
                auto bstring_content = vec_from_file(in, bstring_size);
                // Add to database.
                temp_map[key] = exostore::bstring(bstring_content);
            }
            else if(marker_str == "ZSET")   // Sorted set
            {
                // Read size of zset.
                std::size_t zset_size;
                in.read(reinterpret_cast<char*>(&zset_size), sizeof(zset_size));
                // Read score-member pairs into zset.
                exostore::zset zset;
                for (std::size_t j = 0; j < zset_size; j++)
                {
                    // Read score.
                    double score;
                    in.read(reinterpret_cast<char*>(&score), sizeof(score));
                    // Read member size.
                    std::size_t member_size;
                    in.read(reinterpret_cast<char*>(&member_size),
                        sizeof(member_size));
                    // Read member.
                    auto member = vec_from_file(in, member_size);
                    // Add to zset.
                    zset.add(member, score);
                }
                // Add to database.
                temp_map[key] = zset;
            }
        }
    }
    catch (std::ifstream::failure e)
    {
        throw exostore::load_error("Bad file format");
    }

    map_ = std::move(temp_map);
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
