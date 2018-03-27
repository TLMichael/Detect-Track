cmake_minimum_required(VERSION 3.5)
project(TM_track)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp template.h)

find_package(OpenCV REQUIRED)
add_executable(TM_track ${SOURCE_FILES})
target_link_libraries(TM_track ${OpenCV_LIBS})
