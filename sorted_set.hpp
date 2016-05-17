#ifndef __EXOREDIS_SORTED_SET_HPP__
#define __EXOREDIS_SORTED_SET_HPP__

#include <vector>

class sorted_set
{
public:
    typedef vector<unsigned char> element_type;

    sorted_set();

    bool contains(const element_type&) const;
    bool contains_element_score(const element_type& el, double score) const;

    double get_score(const element_type&) const;

    // Adds the element or updates the score if it exists.
    void add(const element_type& element, double score);
};

#endif
