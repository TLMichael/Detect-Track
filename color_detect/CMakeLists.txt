cmake_minimum_required(VERSION 2.8)

project(fuck)

SET(CMAKE_C_COMPILER g++)
if(CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options(-std=c++11)
message(STATUS "optional:-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()


file(GLOB SRCS ./src/*.cpp)
file(GLOB HDRS ./src/*.h*)

set(traincascade_files ${SRCS} ${HDRS})

add_executable(fuck ${traincascade_files})

target_link_libraries(fuck ${OpenCV_LIBS})
