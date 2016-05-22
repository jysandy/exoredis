add_definitions(-std=c++14)

find_library(BOOST_TEST libboost_unit_test_framework.a)

add_executable(tests tests.cpp ../binary_string.cpp ../sorted_set_key.cpp
    ../sorted_map_key.cpp ../sorted_set.cpp ../exostore.cpp)
target_link_libraries(tests -pthread ${BOOST_TEST})
