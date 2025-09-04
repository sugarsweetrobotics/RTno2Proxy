set(rtno_hal_srcs 
  EtherTcp.cpp
  Serial.cpp
  SerialPort.cpp
  ServerSocket.cpp
  Socket.cpp
)

add_library(rtno_hal  STATIC ${rtno_hal_srcs})
