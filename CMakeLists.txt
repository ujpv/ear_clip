cmake_minimum_required(VERSION 3.8)
project(triangulation)
enable_testing ()

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(tests)
add_subdirectory(ear_clip)
add_subdirectory(gui)

enable_testing()
add_test(NAME unit_test
         COMMAND tests)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS tests)
