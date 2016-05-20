cmake_minimum_required(VERSION 2.8)

project("ExoRedis")

add_definitions(-std=c++14)
add_subdirectory(tests)
find_library(BOOST_SYSTEM libboost_system.a)

# add_executable(exoredis exoredis.cpp)
# target_link_libraries(exoredis -pthread ${BOOST_SYSTEM})
