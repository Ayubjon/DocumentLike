cmake_minimum_required(VERSION 3.15)
project(document_like)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Убедимся, что используется правильный SDK (Были проблемы с маком)
if(APPLE)
    if(NOT CMAKE_OSX_SYSROOT)
        set(CMAKE_OSX_SYSROOT $(xcrun --show-sdk-path))
    endif()
    message(STATUS "Using SDK: ${CMAKE_OSX_SYSROOT}")
endif()

# Убедимся, что используется правильный компилятор и стандартная библиотека C++ (аналогично)
if(APPLE)
    find_program(CMAKE_CXX_COMPILER clang++ REQUIRED HINTS $(xcrun --find clang++))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

file(GLOB SOURCES "src/*.cpp" "src/*.hpp")

add_library(document_like STATIC ${SOURCES})

target_include_directories(document_like PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(test)
