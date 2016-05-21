#ifndef __TEST_SORTED_MAP_KEY_HPP__
#define __TEST_SORTED_MAP_KEY_HPP__

#include "../sorted_map_key.hpp"
#include "../util.hpp"

BOOST_AUTO_TEST_CASE(test_smk_eq)
{
    auto v1 = string_to_vec("some content");
    auto v2 = string_to_vec("some other content");
    auto smk1 = sorted_map_key::create_owned(v1);
    auto smk2 = sorted_map_key::create_unowned(&v1);
    auto smk3 = sorted_map_key::create_owned(v2);
    auto equal_to = sorted_map_key::equal_to();
    BOOST_CHECK(equal_to(smk1, smk2));
    BOOST_CHECK(!equal_to(smk1, smk3));
}

BOOST_AUTO_TEST_CASE(test_smk_hash)
{
    // The hash values should be the same.
    auto v1 = string_to_vec("some content");
    auto v2 = string_to_vec("some other content");
    auto smk1 = sorted_map_key::create_owned(v1);
    auto smk2 = sorted_map_key::create_unowned(&v1);
    auto smk3 = sorted_map_key::create_owned(v2);
    auto hasher = sorted_map_key::hash();
    BOOST_CHECK_EQUAL(hasher(smk1), hasher(smk2));
    BOOST_CHECK_NE(hasher(smk1), hasher(smk3));
}

#endif
