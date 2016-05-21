#ifndef __SORTED_SET_KEY_HPP__
#define __SORTED_SET_KEY_HPP__

#include <vector>

/*
Element in the <set> of the sorted set. Contains a score and a non-owning
pointer to the member.
*/

class sorted_set_key
{
public:
    struct compare
    {
        bool operator()(const sorted_set_key&, const sorted_set_key&);
    };

    sorted_set_key(double score);
    sorted_set_key(double score, std::vector<unsigned char>* ptr);

    double score() const;
    // Throws if member_ptr_ is null.
    const std::vector<unsigned char>& member() const;

    // Constructs a new key with a new score bu the same
    // pointer to member.
    sorted_set_key with_new_score(double score) const;

private:
    double score_;
    std::vector<unsigned char>* member_ptr_;
};

#endif
