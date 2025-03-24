# subprocess/test

cmake_minimum_required(VERSION 3.10)

find_package(GTest REQUIRED)

# add_executable(file_test file_test.cpp)
# add_executable(bytes_test bytes_test.cpp)
add_executable(popen_test popen_test.cpp)
add_executable(streamable_test streamable_test.cpp)

# target_link_libraries(file_test GTest::GTest GTest::Main subprocess)
# target_link_libraries(bytes_test GTest::GTest GTest::Main subprocess)
target_link_libraries(popen_test GTest::GTest GTest::Main subprocess)
target_link_libraries(streamable_test GTest::GTest GTest::Main subprocess)

add_subdirectory(helpers)