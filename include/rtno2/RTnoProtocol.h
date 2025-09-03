#pragma once

#include <stdint.h>
#include <string>
#include <exception>
#include <optional>
#include "Transport.h"
#include "RTnoProfile.h"
#include "Logger.h"
#include "Result.h"


namespace ssr {


  template<typename T>
  inline T into(const uint8_t* buf, const uint8_t size) {
    return *(T*)buf;
  }

  template<typename T>
  inline std::vector<T> into_vec(const uint8_t* buf, const uint8_t size) {
    int length = size / sizeof(T);
    std::vector<T> val;
    for (int i = 0;i < length;i++) {
      val.push_back(into<T>(buf+i*sizeof(T), sizeof(T)));
    }
    return val;
  }

  union intermediate_float_uint8 {
    uint8_t uint8_value[4];
    float float_value;
  };

  template<>
  inline float into<float>(const uint8_t* buf, const uint8_t size) {
    intermediate_float_uint8 i;
    memcpy(i.uint8_value, buf, size);
    return i.float_value;
  }

  union intermediate_double_uint8 {
    uint8_t uint8_value[8];
    double double_value;
  };

  template<>
  inline double into<double>(const uint8_t* buf, const uint8_t size) {
    intermediate_double_uint8 i;
    memcpy(i.uint8_value, buf, size);
    return i.double_value;
  }



  class GetProfileException : public std::exception {
  private:
    std::string msg;
  public:
  GetProfileException(std::string str = "") : msg("GetProfileException:" + str) {}
    virtual ~GetProfileException() throw() {}

    virtual const char* what() const throw() {return msg.c_str();}
  };

  class PacketErrorException : public std::exception {
  private:
    std::string msg;
  public:
  PacketErrorException(std::string str = "") : msg("PacketErrorException:" + str) {}
    virtual ~PacketErrorException() throw() {}

    virtual const char* what() const throw() {return msg.c_str();}
  };


  class ChecksumException : public std::exception {
  private:
    std::string msg;
  public:
  ChecksumException(std::string str = "") : msg("ChecksumException:" + str) {}
    virtual ~ChecksumException() throw() {}

    virtual const char* what() const throw() {return msg.c_str();}
  };

  class TimeoutException : public std::exception {
  private:
    std::string msg;
  public:
  TimeoutException(std::string str = "") : msg("TimeoutException:" + str) {}
    virtual ~TimeoutException() throw() {}

    virtual const char* what() const throw() {return msg.c_str();}
  };



  class ExecuteFailedException : public std::exception {
  private:
    std::string msg;
  public:
  ExecuteFailedException(std::string str = "") : msg("ExecuteFailedException:" + str) {}
    virtual ~ExecuteFailedException() throw() {}

    virtual const char* what() const throw() {return msg.c_str();}
  };

  class UnknownOutPortRequestedException : public std::exception {
  private:
    std::string msg;
  public:
  UnknownOutPortRequestedException(std::string str = "") : msg("UnknownOutPortRequestedException:" + str) {}
    virtual ~UnknownOutPortRequestedException() throw() {}

    virtual const char* what() const throw() {return msg.c_str();}
  };


  struct RTnoState {
    uint8_t status;
    std::string message;

    std::string to_string() const { 
      if (status == STATE_ERROR) {
        std::stringstream ss;
        ss << "ERROR(" << this->message << ")";
        return ss.str();
      }
      return state_to_string(status);
    }
  };


  struct RTnoECType {
    uint8_t ec_type;

    std::string to_string() const { 
      return ec_type_to_string(ec_type);
    }
  };

  class RTnoRTObjectWrapper;

  class RTnoProtocol {
  private:
    Transport *m_pTransport;
    RTnoLogger m_Logger;
  public:
    
  public:
    RTnoProtocol(Transport* pTransport);
    virtual ~RTnoProtocol(void);
    
  public:

    std::optional<RTnoPacket> waitCommand(const COMMAND command, const uint32_t wait_usec);
    RTnoProfile getRTnoProfile(const uint32_t wait_usec, const int retry_count=5);
    RTnoState getRTnoState(const uint32_t wait_usec, const int retry_count=5);
    RTnoECType getRTnoExecutionContextType(const uint32_t wait_usec, const int retry_count=5);

    RTnoState activateRTno(const uint32_t wait_usec, const int retry_count=5);
    RTnoState deactivateRTno(const uint32_t wait_usec, const int retry_count=5);
    RESULT executeRTno(const uint32_t wait_usec, const int retry_count=5);

    RESULT sendData(const std::string& portName, const uint8_t* data, const uint8_t length, const uint32_t wait_usec = 20*1000, const int retry_count=5);

    RESULT receiveData(const std::string& portName, uint8_t* data, const uint8_t max_size, uint8_t* size_read, const uint32_t wait_usec = 20*1000, const int retry_count=5);

    template<typename T>
    ssr::rtno::result_t<T> receiveAs(const std::string& portName) {
      const uint8_t BUFSIZE = 255;
      uint8_t size;
      uint8_t buffer[BUFSIZE];
      auto state = this->receiveData(portName, buffer, BUFSIZE, &size);
      if (state == RESULT::OK) {
        return into<T>(buffer, size);
      }
      return state;
    }

    template<typename T>
    ssr::rtno::result_t<std::vector<T>> receiveSeqAs(const std::string& portName) {
      const uint8_t BUFSIZE = 255;
      uint8_t size;
      uint8_t buffer[BUFSIZE];
      auto state = this->receiveData(portName, buffer, BUFSIZE, &size);
      if (state == RESULT::OK) {
        return into_vec<T>(buffer, size);
      } 
      return state;
    }



  private:
    RTnoProfile onGetProfile(const RTnoPacket& packet);
    // void onAddInPort(const RTnoPacket& packet);
    // void onAddOutPort(const RTnoPacket& packet);
    
    PortProfile parsePortProfile(const RTnoPacket& packet);


  // public:
  //   uint8_t getRTnoStatus();
  // public:
  //   unsigned char getRTnoExecutionContextType();
  //   uint8_t initialize();
  //   uint8_t activate();
  //   uint8_t deactivate();
  //   uint8_t reset();
    
  //   uint8_t sendData(const std::string& portName, const uint8_t* data, const uint8_t length);
  //   int32_t sendExecuteTrigger();

  //   void handleReceivedPacket(const uint32_t wait_usec);
  //   void receiveData(const uint8_t* data);

  //   int SendExecuteTrigger();
    
  //   int ReceiveReturnCode(unsigned char intf);
    

  // private:
  //   std::string GetStringFromPacket(const unsigned char* start_adr, int length);
  };
}
