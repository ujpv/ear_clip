cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

include_directories(../ear_clip)

add_executable(tests main.cpp)

target_link_libraries(tests ear_clip)
