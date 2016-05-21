#include "sorted_map_key.hpp"

#include <boost/functional/hash.hpp>


bool sorted_map_key::equal_to::operator()(const sorted_map_key& left,
    const sorted_map_key& right)
{
    return left.member() == right.member();
}

std::size_t sorted_map_key::hash::operator()(const sorted_map_key& hashee)
{
    return boost::hash_value(hashee.member());
}

sorted_map_key sorted_map_key::create_owned(const std::vector<unsigned char>& v)
{
    return sorted_map_key(v);
}

sorted_map_key sorted_map_key::create_unowned(std::vector<unsigned char>* v)
{
    return sorted_map_key(v);
}

sorted_map_key::sorted_map_key(const std::vector<unsigned char>& v)
    : member_ptr_(new std::vector<unsigned char>(v)),
        unowned_member_ptr_(nullptr)
{
}

sorted_map_key::sorted_map_key(std::vector<unsigned char>* v)
    : unowned_member_ptr_(v)    // member_ptr_ will be nullptr
{
}

const std::vector<unsigned char>& sorted_map_key::member() const
{
    if (unowned_member_ptr_ != nullptr)
    {
        return *unowned_member_ptr_;
    }
    else
    {
        return *member_ptr_;
    }
}

sorted_set_key sorted_map_key::make_set_key(double score) const
{
    if (unowned_member_ptr_ != nullptr)
    {
        return sorted_set_key(score, unowned_member_ptr_);
    }
    else
    {
        return sorted_set_key(score, member_ptr_.get());
    }}
