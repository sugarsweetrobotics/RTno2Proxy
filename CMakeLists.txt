cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)

project(rtno2_proxy)

include_directories(include)
include_directories(include/hal)
include_directories(include/rtno2)

include_directories(thirdparty/spdlog-1.15.3/include)

add_subdirectory(src)