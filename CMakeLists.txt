# subprocess

cmake_minimum_required(VERSION 3.10)

project(subprocess VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)

enable_testing()

add_test(NAME popen_test COMMAND test/popen_test)
add_test(NAME streamable_test COMMAND test/streamable_test)