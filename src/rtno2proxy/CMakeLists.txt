set(rtno_srcs 
  transport.cpp
  packet.cpp
  protocol.cpp
  logger.cpp
)
add_library(rtno_proxy STATIC ${rtno_srcs})
