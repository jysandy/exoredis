#ifndef __TEST_EXOSTORE_HPP__
#define __TEST_EXOSTORE_HPP__

#include <vector>
#include <thread>
#include <chrono>

#include "../exostore.hpp"
#include "../util.hpp"

struct exo_fixture
{
public:
    exo_fixture() : db("some string")
    {
        d1 = string_to_vec("some content");
        d2 = string_to_vec("some other content");
        d3 = string_to_vec("entirely different content");
        k1 = string_to_vec("some key");
        k2 = string_to_vec("some other key");
        k3 = string_to_vec("an entirely different key");
        db.set(k1, exostore::bstring(d1));
        db.set(k2, exostore::bstring(d2));
        exostore::zset zset;
        zset.add(d1, 1.0);
        zset.add(d2, 2.0);
        zset.add(d3, 3.0);
        db.set(k3, zset);
    }

    std::vector<unsigned char> k1, k2, k3;
    std::vector<unsigned char> d1, d2, d3;
    exostore db;
};

BOOST_FIXTURE_TEST_CASE(test_exostore_key_exists, exo_fixture)
{
    BOOST_CHECK(db.key_exists(k1));
    BOOST_CHECK(db.key_exists(k2));
    BOOST_CHECK(db.key_exists(k3));
    auto k4 = string_to_vec("some key");        // This should exist
    BOOST_CHECK(db.key_exists(k4));
    k4 = string_to_vec("nonsense");             // This shouldn't
    BOOST_CHECK(!db.key_exists(k4));
    db.set(k4, exostore::bstring(d1, 1000));     // Should expire in a second
    BOOST_CHECK(db.key_exists(k4));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    BOOST_CHECK(!db.key_exists(k4));
}

BOOST_FIXTURE_TEST_CASE(test_exostore_is_type, exo_fixture)
{
    BOOST_CHECK(db.is_type<exostore::bstring>(k1));
    BOOST_CHECK(db.is_type<exostore::bstring>(k2));
    BOOST_CHECK(db.is_type<exostore::zset>(k3));
    BOOST_CHECK(!db.is_type<exostore::zset>(k2));
}

BOOST_FIXTURE_TEST_CASE(test_exostore_get, exo_fixture)
{
    auto& val = db.get<exostore::bstring>(k1);
    BOOST_CHECK(val.bdata() == d1);

    val = db.get<exostore::bstring>(k2);
    BOOST_CHECK(val.bdata() == d2);

    BOOST_CHECK_THROW(db.get<exostore::bstring>(k3),
        exostore::type_error);

    BOOST_CHECK_THROW(db.get<exostore::bstring>(string_to_vec("hue")),
        exostore::key_error);

    auto& zset = db.get<exostore::zset>(k3);
    BOOST_CHECK(zset.contains_element_score(d1, 1.0));
    BOOST_CHECK(zset.contains_element_score(d2, 2.0));
    BOOST_CHECK(zset.contains_element_score(d3, 3.0));

    BOOST_CHECK_EQUAL(zset.size(), 3);
    auto k4 = string_to_vec("huehue");
    zset.add(k4, 3.0);
    auto& same_zset = db.get<exostore::zset>(k3);
    BOOST_CHECK_EQUAL(zset.size(), 4);
    BOOST_CHECK(zset.contains_element_score(k4, 3.0));
}

#endif
