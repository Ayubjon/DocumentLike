cmake_minimum_required(VERSION 3.15)
project(test_main)

add_executable(test_main main.cpp)

target_include_directories(test_main PRIVATE ${CMAKE_SOURCE_DIR}/../src)

find_package(fmt REQUIRED)

target_link_libraries(test_main PRIVATE fmt::fmt document_like)