#ifndef __TEST_SORTED_SET_KEY_HPP__
#define __TEST_SORTED_SET_KEY_HPP__

#include "../sorted_set_key.hpp"
#include "../util.hpp"
#include <stdexcept>

BOOST_AUTO_TEST_CASE(test_ssk_compare)
{
    // s1 comes before s2, lexicographically (even raw byte-wise)
    std::string s1 = "a string";
    std::string s2 = "a string which is bigger";

    auto v1 = string_to_vec(s1);
    auto v2 = string_to_vec(s2);

    auto k1 = sorted_set_key(2.0);
    auto k2 = sorted_set_key(2.0, &v1);
    auto k3 = sorted_set_key(2.0, &v2);
    auto k4 = sorted_set_key(3.0, &v1);
    auto comparer = sorted_set_key::compare();

    BOOST_CHECK(comparer(k1, k2));
    BOOST_CHECK(comparer(k2, k3));
    BOOST_CHECK(comparer(k2, k4));
    BOOST_CHECK(!comparer(k1, k1));
    BOOST_CHECK(!comparer(k2, k2));
}

BOOST_AUTO_TEST_CASE(test_ssk_accessors)
{
    auto v1 = string_to_vec("a string");
    auto k1 = sorted_set_key(2.0);
    BOOST_CHECK_CLOSE(k1.score(), 2.0, 0.0001);
    BOOST_CHECK_THROW(k1.member(), std::runtime_error);

    auto k2 = sorted_set_key(2.0, &v1);
    BOOST_CHECK(k2.member() == v1);     // CHECK_EQUAL requires operator<< to be defined
}


#endif
