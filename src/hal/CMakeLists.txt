set(rtno_hal_srcs
  EtherTcp.cpp
  Serial.cpp
  SerialPort.cpp
)

add_library(rtno_hal SHARED ${rtno_hal_srcs})
