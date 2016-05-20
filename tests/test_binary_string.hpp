#ifndef __TEST_BINARY_STRING_HPP__
#define __TEST_BINARY_STRING_HPP__

#include <thread>
#include <chrono>
#include <string>

#include "../binary_string.hpp"
#include "../util.hpp"

BOOST_AUTO_TEST_CASE(test_without_expiry)
{
    std::string contents = "some contents";
    auto bstr = binary_string(string_to_vec(contents));
    BOOST_CHECK(!bstr.has_expired());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    BOOST_CHECK(!bstr.has_expired());
    BOOST_CHECK_EQUAL(contents, vec_to_string(bstr.bdata()));
}

BOOST_AUTO_TEST_CASE(test_with_expiry)
{
    std::string contents = "some contents";
    auto bstr = binary_string(string_to_vec(contents), 2000);
    BOOST_CHECK(!bstr.has_expired());
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    BOOST_CHECK(bstr.has_expired());
    BOOST_CHECK_EQUAL(contents, vec_to_string(bstr.bdata()));
}

#endif
