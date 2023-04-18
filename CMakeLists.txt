cmake_minimum_required(VERSION 3.5)

project(affinity)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES affinity.c)

add_executable(affinity ${SOURCE_FILES})

target_link_libraries(affinity pthread)