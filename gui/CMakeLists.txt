cmake_minimum_required(VERSION 3.8)
project(gui)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_AUTOMOC ON)
set(SOURCE_FILES
        main.cpp
        main_window.cpp
        paint_area.cpp
        polygon_builder.cpp
        triangulation.cpp
)

find_package(Qt5Widgets REQUIRED)
include_directories(../ear_clip)

add_executable(gui ${SOURCE_FILES})

target_link_libraries(gui Qt5::Widgets ear_clip)
