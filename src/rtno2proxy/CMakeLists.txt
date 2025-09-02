set(rtno_srcs 
  Transport.cpp
  RTnoPacket.cpp
  RTnoProtocol.cpp
)
add_library(rtno_proxy STATIC ${rtno_srcs})
