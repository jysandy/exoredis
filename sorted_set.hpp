#ifndef __EXOREDIS_SORTED_SET_HPP__
#define __EXOREDIS_SORTED_SET_HPP__

#include <vector>
#include <cstddef>
#include <utility>
#include <set>
#include <boost/unordered_map.hpp>
#include "sorted_set_key.hpp"
#include "sorted_map_key.hpp"


/*
 * Represents a sorted set in the database.
 * It is implemented using both a hash table (boost::unordered_map) and a
 * balanced binary tree (std::set). The hash table maps members to their scores.
 * This is used to ensure uniqueness of the members. The binary tree stores the
 * score-member pairs in the correct sorted order. Both the hash table and the
 * binary tree keys store pointers to the members, so that the members are
 * stored only once in memory.
 *
 * Many of these methods accept a reference to a member and internally create a
 * temporary key using its address. Because of this design, the internal use of
 * pointers is not exposed. When such a temporary key is created, the temporary
 * key should not take ownership of the member.
 *
 * The unordered_map keys (besides the temporary ones) are responsible for
 * managing the memory of the members. The std::set keys only stoe raw
 * non-owning pointers to these managed members.
 */
class sorted_set
{
public:
    typedef std::vector<unsigned char> member_type;
    typedef sorted_set_key set_key_type;
    typedef sorted_map_key map_key_type;
    typedef std::set<set_key_type, set_key_type::compare> set_type;
    typedef boost::unordered_map<map_key_type, double,
        map_key_type::hash, map_key_type::equal_to> map_type;

    sorted_set();

    // Returns true if a member is present, regardless of its score.
    bool contains(const member_type&) const;

    // Returns true if the given score-member pair is present.
    bool contains_element_score(const member_type& m,
        double score) const;

    // Returns 0 if member is not present.
    double get_score(const member_type&) const;

    // Note that the size of the unordered_map and the size of the set
    // should be the same.
    std::size_t size() const;

    // Adds the element or updates the score if it exists.
    void add(const member_type& element, double score);

    // Number of elements with min <= score <= max.
    std::size_t count(double min, double max) const;

    // Returns iterators to the input indices. Both indices are inclusive.
    // The end iterator returned is exclusive.
    std::pair<set_type::const_iterator, set_type::const_iterator>
        element_range(std::size_t start, std::size_t end) const;

private:
    set_type set_;
    map_type map_;
};

#endif
