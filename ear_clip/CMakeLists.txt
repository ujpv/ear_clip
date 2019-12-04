cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_LIB ear_clip.cpp ear_clip.h)

add_library(ear_clip STATIC ${SOURCE_LIB})
