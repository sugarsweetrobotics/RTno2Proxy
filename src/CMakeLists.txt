set(rtno_main_srcs rtno_main.cpp)
set(com2tcp_main_srcs com2tcp_main.cpp)

add_subdirectory(hal)
add_subdirectory(rtno2proxy)

add_executable(rtno2 ${rtno_main_srcs})
target_link_libraries(rtno2 rtno_hal rtno_proxy)


add_executable(com2tcp ${com2tcp_main_srcs})
target_link_libraries(com2tcp rtno_hal )
