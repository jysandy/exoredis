#ifndef __EXOREDIS_SORTED_SET_HPP__
#define __EXOREDIS_SORTED_SET_HPP__

#include <vector>
#include <cstddef>
#include "sorted_set_key.hpp"

class sorted_set
{
public:
    typedef vector<unsigned char> element_type;
    typedef sorted_set_key key_type;

    sorted_set();   // Default score is 0

    bool contains(const element_type&) const;
    bool contains_element_score(const element_type& el, double score) const;

    double get_score(const element_type&) const;
    std::size_t size() const;

    // Adds the element or updates the score if it exists.
    void add(const element_type& element, double score);

    // Number of elements with min <= score <= max.
    std::size_t count(double min, double max) const;
};

#endif
