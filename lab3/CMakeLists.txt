cmake_minimum_required(VERSION 3.17)
project(lab3)

set(CMAKE_CXX_STANDARD 14)
set(SOURCE_LIB sanitizer.cpp)
set(CMAKE_CXX_FLAGS -pthread)


add_library(sanitizer SHARED ${SOURCE_LIB})
add_executable(lab3 main.cpp)

target_link_libraries(lab3 sanitizer)

