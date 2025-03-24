# subprocess/src

# add_library (
#     subprocess STATIC
#     bytes.cpp 
#     file.cpp
#     popen.cpp 
# )

# target_include_directories(subprocess PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library (
    subprocess STATIC
    bytes.cpp
    popen.cpp
    streamable.cpp
    types.cpp
)

target_include_directories(subprocess PUBLIC ${CMAKE_SOURCE_DIR}/include)