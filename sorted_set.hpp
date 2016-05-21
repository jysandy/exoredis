#ifndef __EXOREDIS_SORTED_SET_HPP__
#define __EXOREDIS_SORTED_SET_HPP__

#include <vector>
#include <cstddef>
#include <utility>
#include <set>
#include <boost/unordered_map.hpp>
#include "sorted_set_key.hpp"
#include "sorted_map_key.hpp"

class sorted_set
{
public:
    typedef vector<unsigned char> member_type;
    typedef sorted_set_key set_key_type;
    typedef sorted_map_key map_key_type;
    typedef std::set<set_key_type> set_type;
    typedef boost::unordered_map<map_key_type, double,
        map_key_type::hash, map_key_type::equal_to> map_type;

    sorted_set();   // Default score is 0

    bool contains(const member_type&) const;
    bool contains_element_score(const member_type& el, double score) const;

    double get_score(const member_type&) const;

    // Note that the size of the unordered_map and the size of the set
    // should be the same.
    std::size_t size() const;

    // Adds the element or updates the score if it exists.
    void add(const member_type& element, double score);

    // Number of elements with min <= score <= max.
    std::size_t count(double min, double max) const;

    // Iterators to the passed indices. Both indices are inclusive.
    // The end iterator though is exclusive.
    std::pair<set_type::const_iterator, set_type::const_iterator>
        element_range(std::size_t start, std::size_t end);

private:
    set_type set_;
    map_type map_;
};

#endif
