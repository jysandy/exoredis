#ifndef __EXOREDIS_SORTED_SET_HPP__
#define __EXOREDIS_SORTED_SET_HPP__

#include <vector>
#include <cstddef>
#include <utility>
#include <set>
#include "sorted_set_key.hpp"

class sorted_set
{
public:
    typedef vector<unsigned char> element_type;
    typedef sorted_set_key key_type;
    typedef std::set<sorted_set_key> set_type;

    sorted_set();   // Default score is 0

    bool contains(const element_type&) const;
    bool contains_element_score(const element_type& el, double score) const;

    double get_score(const element_type&) const;

    // Note that the size of the unordered_map and the size of the set
    // should be the same.
    std::size_t size() const;

    // Adds the element or updates the score if it exists.
    void add(const element_type& element, double score);

    // Number of elements with min <= score <= max.
    std::size_t count(double min, double max) const;

    // Iterators to the passed indices. Both indices are inclusive.
    // The end iterator though is exclusive.
    std::pair<set_type::const_iterator, set_type::const_iterator>
        element_range(std::size_t start, std::size_t end);
};

#endif
