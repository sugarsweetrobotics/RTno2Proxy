#pragma once

#include <stdint.h>
#include <string>
#include <exception>
#include <optional>
#include "Transport.h"
#include "RTnoProfile.h"



namespace ssr {
  class GetProfileException : public std::exception {
  private:
    std::string msg;
  public:
  GetProfileException(std::string str = "") : msg("GetProfileException:" + str) {}
    virtual ~GetProfileException() throw() {}

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
  public:
    
  public:
    RTnoProtocol(Transport* pTransport);
    virtual ~RTnoProtocol(void);
    
  public:

    std::optional<RTnoPacket> waitCommand(const COMMAND command, const uint32_t wait_usec);
    RTnoProfile getRTnoProfile(const uint32_t wait_usec);
    RTnoState getRTnoState(const uint32_t wait_usec);
    RTnoECType getRTnoExecutionContextType(const uint32_t wait_usec);

    RTnoState activateRTno(const uint32_t wait_usec);
    RTnoState deactivateRTno(const uint32_t wait_usec);
    RTnoState executeRTno(const uint32_t wait_usec);

    RTnoState sendData(const std::string& portName, const uint8_t* data, const uint8_t length);

    RTnoState receiveData(const std::string& portName, uint8_t* data, const uint8_t max_size, uint8_t* size_read);
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
