#include "sorted_set.hpp"

#include <iterator>
#include <stdexcept>

sorted_set::sorted_set()
{
}

bool sorted_set::contains(const sorted_set::member_type& m) const
{
    auto temp_key = sorted_set::map_key_type::create_unowned(&m);
    return map_.count(temp_key) != 0;
}

bool sorted_set::contains_element_score(
    const sorted_set::member_type& m,
    double score) const
{
    auto temp_key = sorted_set::map_key_type::create_unowned(&m);
    return map_.count(temp_key) != 0 && map_.at(temp_key) == score;
}

double sorted_set::get_score(const sorted_set::member_type& m) const
{
    auto temp_key = sorted_set::map_key_type::create_unowned(&m);
    if (map_.count(temp_key) == 0)
    {
        return 0;
    }
    else
    {
        return map_.at(temp_key);
    }
}

std::size_t sorted_set::size() const
{
    return map_.size();
}

void sorted_set::add(const sorted_set::member_type& m,
    double score)
{
    auto temp_key = sorted_set::map_key_type::create_unowned(&m);

    if (map_.count(temp_key) != 0)
    {
        // We want to avoid copying the member by making an
        // owned key if it already exists in the map.
        auto old_score = map_[temp_key];
        map_[temp_key] = score;
        // Get rid of the key from the set.
        auto temp_set_key = sorted_set::set_key_type(old_score, &m);
        auto old_set_key = set_.find(temp_set_key);
        if (old_set_key == set_.end())
        {
            throw std::runtime_error("Old set key not found");
        }
        set_.insert(old_set_key->with_new_score(score));
        set_.erase(old_set_key);
    }
    else
    {
        // Make new keys.
        auto new_map_key = sorted_set::map_key_type::create_owned(m);
        auto new_set_key = new_map_key.make_set_key(score);
        map_[new_map_key] = score;
        set_.insert(new_set_key);
    }
}

std::size_t sorted_set::count(double min, double max) const
{
    auto temp_lower_key = sorted_set::set_key_type(min);
    auto temp_upper_key = sorted_set::set_key_type(max, true);

    auto lb = set_.lower_bound(temp_lower_key);
    auto ub = set_.upper_bound(temp_upper_key);

    return std::distance(lb, ub);
}

std::pair<sorted_set::set_type::const_iterator, sorted_set::set_type::const_iterator>
    sorted_set::element_range(std::size_t start, std::size_t end) const
{
    auto first = set_.cbegin();
    std::advance(first, start);
    auto second = set_.cbegin();
    std::advance(second, end);
    return std::pair<set_type::const_iterator, set_type::const_iterator>(
        first, second
    );
}
