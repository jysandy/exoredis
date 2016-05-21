#ifndef __TEST_SORTED_SET_HPP__
#define __TEST_SORTED_SET_HPP__

#include "../sorted_set.hpp"
#include "../util.hpp"

#include <vector>

struct F
{
    F()
    {
        v1 = string_to_vec("some content");
        v2 = string_to_vec("some other content");
        v3 = string_to_vec("some more content");
        v4 = string_to_vec("entirely different content");

        zset.add(v2, 2.0);
        zset.add(v3, 3.0);
        zset.add(v1, 1.0);
    }

    std::vector<unsigned char> v1, v2, v3, v4;
    sorted_set zset;
};

BOOST_FIXTURE_TEST_CASE(test_sorted_set_contains, F)
{
    BOOST_CHECK(zset.contains(v1));
    BOOST_CHECK(zset.contains(v2));
    BOOST_CHECK(zset.contains(v3));
    BOOST_CHECK(!zset.contains(v4));

    BOOST_CHECK(zset.contains_element_score(v1, 1.0));
    BOOST_CHECK(zset.contains_element_score(v2, 2.0));
    BOOST_CHECK(zset.contains_element_score(v3, 3.0));
    BOOST_CHECK(!zset.contains_element_score(v4, 2.0));
}

BOOST_FIXTURE_TEST_CASE(test_sorted_set_get_score, F)
{
    BOOST_CHECK_EQUAL(zset.get_score(v1), 1.0);
    BOOST_CHECK_EQUAL(zset.get_score(v2), 2.0);
    BOOST_CHECK_EQUAL(zset.get_score(v3), 3.0);
    BOOST_CHECK_EQUAL(zset.get_score(v4), 0.0);
    BOOST_CHECK(!zset.contains(v4));
}

 BOOST_FIXTURE_TEST_CASE(test_sorted_set_size, F)
 {
     BOOST_CHECK_EQUAL(zset.size(), 3);
     zset.add(v4, 3.0);
     BOOST_CHECK_EQUAL(zset.size(), 4);
 }

BOOST_FIXTURE_TEST_CASE(test_sorted_set_add, F)
{
    zset.add(v4, 3.5);
    zset.add(v1, 0.5);
    BOOST_CHECK_EQUAL(zset.size(), 4);
    BOOST_CHECK(zset.contains_element_score(v4, 3.5));
    BOOST_CHECK(zset.contains_element_score(v1, 0.5));
}

BOOST_FIXTURE_TEST_CASE(test_sorted_set_count, F)
{
    BOOST_CHECK_EQUAL(zset.count(-1.0, 5.0), 3);
    BOOST_CHECK_EQUAL(zset.count(2.0, 2.0), 1);
    BOOST_CHECK_EQUAL(zset.count(1.0, 2.0), 2);
}

BOOST_FIXTURE_TEST_CASE(test_sorted_set_element_range, F)
{
    std::vector<std::vector<unsigned char>> expected{v1, v2, v3};
    std::vector<std::vector<unsigned char>> actual;
    auto its = zset.element_range(0, 3);
    for (auto it = its.first; it != its.second; it++)
    {
        actual.push_back(it->member());
    }

    BOOST_CHECK(actual == expected);

    zset.add(v4, 1.5);
    std::vector<std::vector<unsigned char>> expected2{v1, v4, v2, v3};
    its = zset.element_range(0, 4);
    actual.clear();
    for (auto it = its.first; it != its.second; it++)
    {
        actual.push_back(it->member());
    }
    BOOST_CHECK(actual == expected2);
}

#endif
