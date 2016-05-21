#include "sorted_set_key.hpp"

#include <algorithm>
#include <stdexcept>

// Keys are ordered by scores first and then by lexicographical order of the
// members.
// Returns true if left is 'less' than right, i.e., left should appear before
// right in the sorted set.
bool sorted_set_key::compare::operator()(const sorted_set_key& left,
    const sorted_set_key& right) const
{
    if (left.score_ == right.score_)
    {
        if (left.lex_largest_)
        {
            return false;
        }

        if (right.member_ptr_ == nullptr)           // Left may be null
        {
            return false;
        }
        else if (left.member_ptr_ == nullptr)       // Right is not null
        {
            return true;
        }
        else    // Both are not null
        {
            return std::lexicographical_compare(
                left.member_ptr_->begin(), left.member_ptr_->end(),
                right.member_ptr_->begin(), right.member_ptr_->end()
            );
        }
    }
    else
    {
        return left.score_ < right.score_;
    }
}

sorted_set_key::sorted_set_key(double score, double lex_largest)
    : score_(score), member_ptr_(nullptr), lex_largest_(lex_largest)
{
}

sorted_set_key::sorted_set_key(double score, const std::vector<unsigned char>* ptr)
    : score_(score), member_ptr_(ptr), lex_largest_(false)
{
}

double sorted_set_key::score() const
{
    return score_;
}

const std::vector<unsigned char>& sorted_set_key::member() const
{
    if (member_ptr_ == nullptr)
    {
        throw std::runtime_error("member_ptr_ of sorted_set_key is null");
    }

    return *member_ptr_;
}

sorted_set_key sorted_set_key::with_new_score(double score) const
{
    return sorted_set_key(score, member_ptr_);
}
