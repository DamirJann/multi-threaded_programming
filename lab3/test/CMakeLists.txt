cmake_minimum_required(VERSION 3.19)
project(test)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_SHARED_LINKER_FLAGS -ldl)
set(SOURCE_LIB ../sanitizer.cpp)

add_library(sanitizer SHARED ${SOURCE_LIB})
add_executable(test main.cpp)

target_link_libraries(test sanitizer)
target_link_libraries(test ${CMAKE_DL_LIBS})