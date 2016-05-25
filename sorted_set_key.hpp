#ifndef __SORTED_SET_KEY_HPP__
#define __SORTED_SET_KEY_HPP__

#include <vector>

/*
 * Element in the <set> of the sorted set. Contains a score and a non-owning
 * pointer to the member.
*/

class sorted_set_key
{
public:
    struct compare
    {
        bool operator()(const sorted_set_key&, const sorted_set_key&) const;
    };

    sorted_set_key(double score, double lex_largest=false);
    sorted_set_key(double score, const std::vector<unsigned char>* ptr);

    double score() const;

    // Throws if member_ptr_ is null.
    const std::vector<unsigned char>& member() const;

    // Constructs a new key with a new score but the same
    // pointer to member.
    sorted_set_key with_new_score(double score) const;

private:
    double score_;
    const std::vector<unsigned char>* member_ptr_;
    // Flag to set whether the null string should be lexicographically
    // the 'largest'. This is useful when finding an inclusive upper_bound
    // based only on score.
    bool lex_largest_;
};

#endif
