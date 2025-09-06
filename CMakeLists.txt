cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20)

project(rtno2_proxy)

if (WIN32)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else (WIN32)


endif (WIN32)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/hal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/rtno2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/spdlog-1.15.3/include)


set(RTNO2PROXY_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/hal ${CMAKE_CURRENT_SOURCE_DIR}/include/rtno2 ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/spdlog-1.15.3/include PARENT_SCOPE)

add_subdirectory(src)