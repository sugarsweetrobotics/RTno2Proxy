set(rtno_main_srcs rtno_main.cpp)


add_subdirectory(hal)
add_subdirectory(rtno2proxy)

add_executable(rtno2 ${rtno_main_srcs})
target_link_libraries(rtno2 rtno_hal rtno_proxy)
