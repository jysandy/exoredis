cmake_minimum_required(VERSION 2.8)

project("ExoRedis")

add_definitions(-std=c++14)
add_subdirectory(tests)
find_library(BOOST_SYSTEM libboost_system.a)

add_executable(exoredis binary_string.cpp db_session.cpp exoredis.cpp
    exostore.cpp sorted_map_key.cpp sorted_set.cpp sorted_set_key.cpp util.cpp)
target_link_libraries(exoredis -pthread ${BOOST_SYSTEM})
