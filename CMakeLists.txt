cmake_minimum_required(VERSION 3.0)

project(ThreadPool)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-std=c++23 -Wall -g)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_library(threadpool SHARED ${SRC_LIST})

add_executable(threadpool_test ${SRC_LIST} test.cpp)

target_link_libraries(threadpool_test threadpool)
