# subprocess/src/CMakeLists.txt

add_library(subprocess STATIC
    bytes.cpp
    popen.cpp
    streamable.cpp
    types.cpp
)

target_include_directories(subprocess PUBLIC ${PROJECT_SOURCE_DIR}/include)