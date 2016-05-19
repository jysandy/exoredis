#ifndef __SORTED_SET_KEY_HPP__
#define __SORTED_SET_KEY_HPP__

#include <vector>

class sorted_set_key
{
public:
    sorted_set_key(double score, std::vector<unsigned char>* ptr);

    double score() const;
    const std::vector<unsigned char>& member() const;
};

#endif
