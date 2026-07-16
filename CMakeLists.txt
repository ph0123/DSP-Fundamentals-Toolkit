cmake_minimum_required(VERSION 3.16)
project(dsp_toolkit CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra -O2)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(demo_spectrogram src/demo_spectrogram.cpp)
add_executable(demo_filter      src/demo_filter.cpp)
add_executable(demo_aec         src/demo_aec.cpp)