#pragma once

#include <string>
#include <sstream>

#include "Result.h"
// Return Values
#define TIMEOUT 1
#define DATA_TIMEOUT 2
#define CHECKSUM_ERROR 3
#define INVALID_PACKET_INTERFACE 65
#define INVALID_PACKET_DATASIZE  66

// Protocol


// Protocol

enum class COMMAND : char {
  INITIALIZE = 'I',
  ACTIVATE = 'A',
  DEACTIVATE = 'D',
  GET_STATE = 'X',

  GET_CONTEXT_TYPE = 'B',
  EXECUTE = 'E',
  RESET = 'R',

  ONERROR = 'C',
  GET_PROFILE = 'Z',
  INPORT_PROFILE = 'P',
  OUTPORT_PROFILE = 'Q',

  SEND_DATA = 'S',
  RECEIVE_DATA = 'G',
  
  PACKET_ERROR = 'F',
  PACKET_ERROR_CHECKSUM = 'H',
  PACKET_ERROR_TIMEOUT = 'K',
};

inline std::string command_to_string(COMMAND cmd) {
  switch (cmd) {
    case COMMAND::INITIALIZE: return "COMMAND::INITIALIZE";
    case COMMAND::ACTIVATE: return "COMMAND::ACTIVATE";
    case COMMAND::DEACTIVATE: return "COMMAND::DEACTIVATE";
    case COMMAND::GET_STATE: return "COMMAND::GET_STATE";
    case COMMAND::GET_CONTEXT_TYPE: return "COMMAND::GET_CONTEXT_TYPE";
    case COMMAND::EXECUTE: return "COMMAND::EXECUTE";
    case COMMAND::RESET: return "COMMAND::RESET";
    case COMMAND::ONERROR: return "COMMAND::ONERROR";
    case COMMAND::GET_PROFILE: return "COMMAND::GET_PROFILE";
    case COMMAND::INPORT_PROFILE: return "COMMAND::OUTPORT_PROFILE";
    case COMMAND::OUTPORT_PROFILE: return "COMMAND::OUTPORT_PROFILE";
    case COMMAND::SEND_DATA: return "COMMAND::SEND_DATA";
    case COMMAND::RECEIVE_DATA: return "COMMAND::RECEIVE_DATA";
    case COMMAND::PACKET_ERROR: return "COMMAND::PACKET_ERROR";
    case COMMAND::PACKET_ERROR_CHECKSUM: return "COMMAND::PACKET_ERROR_CHECKSUM";
    case COMMAND::PACKET_ERROR_TIMEOUT: return "COMMAND::PACKET_ERROR_TIMEOUT";
    default: {
      std::stringstream ss;
      ss << "COMMAND::UNKNOWN(" << (char)cmd << "/" << (int)cmd << ")";
      return ss.str();
    }
  }
}


// // Interface
// #define INITIALIZE 'I'
// #define RTNO_ACTIVATE 'A'
// #define RTNO_DEACTIVATE 'D'
// #define RTNO_EXECUTE 'E'
// #define ONERROR 'C'
// #define RTNO_RESET 'R'
// #define GET_STATUS 'X'
// #define SEND_DATA 'S'
// #define RECEIVE_DATA 'V'
// #define GET_PROFILE 'Z'
// #define GET_CONTEXT 'B'


// #define ADD_INPORT 'P'
// #define ADD_OUTPORT 'Q'

// #define INPORT_ISNEW 'N'
// #define INPORT_READ  'J'

// #define RTNO_OK '@'
// #define RTNO_ERROR 'x'

// #define PACKET_ERROR 'F'
// #define PACKET_ERROR_CHECKSUM 'H'
// #define PACKET_ERROR_TIMEOUT 'K'

// #define OUTPORT_WRITE 'W'

// state
enum {
  STATE_CREATED='C',
  STATE_INACTIVE='I',
  STATE_ACTIVE='A',
  STATE_ERROR='E',
  STATE_NONE='N',
};

inline std::string state_to_string(uint8_t state) {
  switch (state) {
    case STATE_CREATED: return "CREATED";
    case STATE_INACTIVE: return "INACTIVE";
    case STATE_ACTIVE: return "ACTIVE";
    case STATE_ERROR: return "ERROR";
    case STATE_NONE: return "NONE";
    default: {
      std::stringstream ss;
      ss << "UNKNOWN(" << (int)state << ")";
      return ss.str();
    }
  }
}


// Communication Settings
#define PACKET_WAITING_TIME 3000 // ms
#define PACKET_SENDING_DELAY 10 // us
#define PACKET_WAITING_DELAY 100 //us
#define PACKET_WAITING_COUNT (PACKET_WAITING_TIME*1000/PACKET_WAITING_DELAY)

#define TYPECODE_TIMED_BOOLEAN 'b'
#define TYPECODE_TIMED_OCTET 'o'
#define TYPECODE_TIMED_CHAR  'c'

#define TYPECODE_TIMED_LONG 'l'
#define TYPECODE_TIMED_FLOAT 'f'
#define TYPECODE_TIMED_DOUBLE 'd'

#define TYPECODE_TIMED_BOOLEAN_SEQ 'B'
#define TYPECODE_TIMED_OCTET_SEQ 'O'
#define TYPECODE_TIMED_CHAR_SEQ 'C'

#define TYPECODE_TIMED_LONG_SEQ 'L'
#define TYPECODE_TIMED_FLOAT_SEQ 'F'
#define TYPECODE_TIMED_DOUBLE_SEQ 'D'

inline std::string typecode_to_str(char typecode) {
  switch (typecode) {
    case TYPECODE_TIMED_BOOLEAN: return "TimedBoolean";
    case TYPECODE_TIMED_CHAR: return "TimedChar";
    case TYPECODE_TIMED_LONG: return "TimedLong";
    case TYPECODE_TIMED_FLOAT: return "TimedFloat";
    case TYPECODE_TIMED_DOUBLE: return "TimedDouble";
    case TYPECODE_TIMED_LONG_SEQ: return "TimedLongSeq";
    case TYPECODE_TIMED_FLOAT_SEQ: return "TimedFloatSeq";
    case TYPECODE_TIMED_DOUBLE_SEQ: return "TimedDoubleSeq";
    default: return "UnknownTypeCode";
  }
}

#define MAX_PACKET_SIZE 64

/***********************************************
 *
 **********************************************/
#define ConnectionTypeSerial1 0x01
#define ConnectionTypeSerial2 0x02
#define ConnectionTypeSerial3 0x03

#define ProxySynchronousExecutionContext 0x21
#define Timer1ExecutionContext 0x22
#define Timer2ExecutionContext 0x23
#define FSPTimerExecutionContext 0x24

#include <iostream>

inline std::string ec_type_to_string(uint8_t ec_type) {
  std::cout << "ec_type_to_string(" << (int)ec_type << ")" << std::endl;
  switch (ec_type) {
    case ProxySynchronousExecutionContext: return "ProxySynchronousExecutionContext";
    case Timer1ExecutionContext: return "Timer1ExecutionContext";
    case Timer2ExecutionContext: return "Timer2ExecutionContext";
    case FSPTimerExecutionContext: return "FSPTimerExecutionContext";
    default: {
      std::stringstream ss;
      ss << "UnknownExecutionContext(" << (int)ec_type << ")";
      return ss.str();
    }
  }
}