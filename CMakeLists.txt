cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20)

project(rtno2_proxy)

if (WIN32)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else (WIN32)


endif (WIN32)

include_directories(include)
include_directories(include/hal)
include_directories(include/rtno2)

include_directories(thirdparty/spdlog-1.15.3/include)

add_subdirectory(src)