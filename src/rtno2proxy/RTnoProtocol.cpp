
#include "rtno2/Packet.h"
#include "rtno2/RTnoProtocol.h"

#include <chrono>
#include <thread>
#include <iostream>

#include "rtno2/Logger.h"


using namespace ssr;


RTnoProtocol::RTnoProtocol(Transport *pTransport) :
  m_pTransport(pTransport), m_Logger(getLogger("RTnoProtocol")) {

  setLogLevel(&m_Logger, ssr::LOGLEVEL::ERROR);
  RTNO_TRACE(m_Logger, "RTnoProtocol() called");
}

RTnoProtocol::~RTnoProtocol(void)
{
}

std::optional<RTnoPacket> RTnoProtocol::waitCommand(const COMMAND command, const uint32_t wait_usec) {
  RTNO_TRACE(m_Logger, "RTnoProtocol::waitCommand({}, {}) called", command_to_string(command), wait_usec);
  if(m_pTransport->isNew(wait_usec)) {
    RTnoPacket pac = m_pTransport->receive(wait_usec);
    RTNO_DEBUG(m_Logger, "waitCommand received {}", pac.to_string());
    if(pac.getInterface() == command) {
      RTNO_TRACE(m_Logger, "RTnoProtocol::waitCommand() exit with success");
      return pac;
    } else if (pac.getInterface() == COMMAND::PACKET_ERROR) {
      auto data_length = pac.getDataLength();
      auto data = pac.getData()[0];
      RTNO_DEBUG(m_Logger, "RTnoProtocol::waitCommand() receive packet_error. data is {}/{}", data, (int)data);
    } else {
      RTNO_DEBUG(m_Logger, "RTnoProtocol::waitCommand() still wait for unknown command({}/{})", ((char)pac.getInterface()), ((int)pac.getInterface()));
    }
  }
  RTNO_TRACE(m_Logger, "RTnoProtocol::waitCommand() exit");
  return std::nullopt;
}


RTnoState RTnoProtocol::getRTnoState(uint32_t wait_usec, const int retry_count) {
  RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoState() called.");
  static const RTnoPacket cmd_packet(COMMAND::GET_STATE);
  for(int i = 0;i < retry_count;i++) {
    try {
      m_pTransport->send(cmd_packet);
      auto result = waitCommand(COMMAND::GET_STATE, wait_usec);
      if (result) {

        RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoState() exit");
        return RTnoState{
          .status = result.value().getData()[0],
        };
      }
    } catch (TimeOutException& ex) {
      RTNO_DEBUG(m_Logger, "In getRTnoState(), timeout. retry");
    }
  }

  RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoState() exit");
  return RTnoState{
    .status = 255
  };
}

RTnoECType RTnoProtocol::getRTnoExecutionContextType(uint32_t wait_usec, const int retry_count) {
  RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoExecutionContextType() called.");
  static const RTnoPacket cmd_packet(COMMAND::GET_CONTEXT_TYPE);
  for(int i = 0;i < retry_count;i++) {
    try {
      m_pTransport->send(cmd_packet);
      auto result = waitCommand(COMMAND::GET_CONTEXT_TYPE, wait_usec);
      if (result) {
        RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoExecutionContextType() exit");
        return RTnoECType{
          .ec_type = result.value().getData()[0],
        };
      }
    } catch (TimeOutException& ex) {
      RTNO_DEBUG(m_Logger, "In getRTnoExecutionContextType() timeout. retry");
    }
  }
  RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoExecutionContextType() exit");
  return RTnoECType {
    .ec_type = 255
  };
}


RTnoProfile RTnoProtocol::getRTnoProfile(const uint32_t wait_usec, const int retry_count) {
  RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoProfile() called.");
  static const RTnoPacket cmd_packet(COMMAND::GET_PROFILE);
  for(int i = 0;i < retry_count;i++) {
    try {
      m_pTransport->send(cmd_packet);
      RTnoProfile profile;

      int timeout_count = wait_usec/1000;
      while(1) {
        if(!m_pTransport->isNew(wait_usec)) {

          RTNO_WARN(m_Logger, "RTnoProtocol::getRTnoProfile() isNew timeout.");
          throw TimeOutException();
        }

        RTnoPacket pac = m_pTransport->receive(wait_usec);
        switch((COMMAND)pac.getInterface()) {
        case COMMAND::GET_PROFILE: // Return Code.
          RTNO_DEBUG(m_Logger, "COMMAND::GET_PROFILE received ({})", profile.to_string());
          RTNO_TRACE(m_Logger, "RTnoProtocol::getRTnoProfile() exit");
          return profile; // onGetProfile(pac);
        
        case COMMAND::INPORT_PROFILE: 
          {
            auto inport_prof = parsePortProfile(pac);
            RTNO_DEBUG(m_Logger, "COMMAND::INPORT_PROFILE received ({})", inport_prof.to_string());
            profile.appendInPort(inport_prof);
          }
          break;

        case COMMAND::OUTPORT_PROFILE:
          {
            auto outport_prof = parsePortProfile(pac);
            RTNO_DEBUG(m_Logger, "COMMAND::OUTPORT_PROFILE received ({})", outport_prof.to_string());
            profile.appendOutPort(outport_prof);
          }
          break;

        case COMMAND::PACKET_ERROR:
        {
          RTNO_ERROR(m_Logger, "COMMAND::PACKET_ERROR received.");
          throw PacketErrorException();
        }
        case COMMAND::PACKET_ERROR_CHECKSUM:
          RTNO_ERROR(m_Logger, "COMMAND::PACKET_ERROR_CHECKSUM received.");
          throw ChecksumException();
        case COMMAND::PACKET_ERROR_TIMEOUT:

          RTNO_ERROR(m_Logger, "COMMAND::PACKET_ERROR_TIMEOUT received.");
          throw TimeOutException();
        default: 
          RTNO_ERROR(m_Logger, "Unknown Command ({})", command_to_string(pac.getInterface()));
        }
      }
    } catch (TimeOutException& ex) {          
      RTNO_WARN(m_Logger, "RTnoProtocol::getRTnoProfile() timeout retry.");
    }
  }
  throw TimeOutException();
}

// RTnoProfile RTnoProtocol::onGetProfile(const RTnoPacket& packet)
// {
//   if (packet.getResult() != RESULT::OK) {
//     std::cout << "--RTnoProtocol::getRTnoProfile() Failed." << std::endl;
//     throw GetProfileException();
//   }
// }

PortProfile RTnoProtocol::parsePortProfile(const RTnoPacket& packet) {
  char strbuf[64];
  memcpy(strbuf, packet.getData()+1, packet.getDataLength()-1);
  strbuf[packet.getDataLength()-1] = 0;
  return PortProfile(packet.getData()[0], strbuf);
}


static bool is_ok(const ssr::RTnoPacket& packet) {
  return packet.getResult() == RESULT::OK;
}

RTnoState RTnoProtocol::activateRTno(const uint32_t wait_usec, const int retry_count) {
  RTNO_TRACE(m_Logger, " - RTnoProtocol::activateRTno() called.");
  static const RTnoPacket cmd_packet(COMMAND::ACTIVATE);
  for(int i = 0;i < retry_count;i++) {
    try {
      m_pTransport->send(cmd_packet);
      auto result = waitCommand(COMMAND::ACTIVATE, wait_usec);
      if (result) {
        RTNO_DEBUG(m_Logger, " - RTnoProtocol::activateRTno() exit with success.");
        return getRTnoState(wait_usec);
      }
      RTNO_DEBUG(m_Logger, " - RTnoProtocol::activateRTno() exit with failure.");
      
    } catch (TimeOutException& ex) {
      RTNO_DEBUG(m_Logger, "In activateRTno, timeout. retry");
    }
  }
  return RTnoState {
    .status=255,
  };
}

RTnoState RTnoProtocol::deactivateRTno(const uint32_t wait_usec, const int retry_count) {
  static const RTnoPacket cmd_packet(COMMAND::DEACTIVATE);
  for(int i = 0;i < retry_count;i++) {
    try {
      m_pTransport->send(cmd_packet);
      auto result = waitCommand(COMMAND::DEACTIVATE, wait_usec);
      if (result) {
        RTNO_DEBUG(m_Logger, " - RTnoProtocol::deactivateRTno() exit with success.");
        return getRTnoState(wait_usec);
      }
    } catch (TimeOutException& ex) {
      RTNO_DEBUG(m_Logger, "In deactivateRTno, timeout. retry");
    }
  }
  return RTnoState {
    .status=255,
  };
}

RESULT RTnoProtocol::executeRTno(const uint32_t wait_usec, const int retry_count) {
  RTNO_TRACE(m_Logger, "executeRTno() called");
  static const RTnoPacket cmd_packet(COMMAND::EXECUTE);
  for(int i = 0;i < retry_count;i++) {
    try {
      m_pTransport->send(cmd_packet);
      auto result = waitCommand(COMMAND::EXECUTE, wait_usec);
      if (result) {
        RTNO_DEBUG(m_Logger, "executeRTno() exit with {}", result_to_string(result.value().getResult()));
        return result.value().getResult();
      }
    } catch (TimeOutException& ex) {      
      RTNO_DEBUG(m_Logger, "In executeRTno, timeout. retry");
    }
  }
  return RESULT::ERROR;
}


RESULT RTnoProtocol::sendData(const std::string& portName, const uint8_t* data, const uint8_t length, const uint32_t wait_usec, const int retry_count) {
  
  RTNO_TRACE(m_Logger, "sendData(port={}, retry={}) called", portName, retry_count);
  int namelen = portName.length();//strlen(portName);
  uint8_t buffer[64];
  buffer[0] = namelen;
  buffer[1] = length;
  memcpy(buffer+2, portName.c_str(), namelen);
  memcpy(buffer+2+namelen, data, length);
  RTnoPacket packet(COMMAND::SEND_DATA, buffer, (RESULT)0, 2 + namelen + length);
  m_pTransport->send(packet);
  auto result = waitCommand(COMMAND::SEND_DATA, 20*1000);
  if (result) {
    RTNO_TRACE(m_Logger, "sendData() exit with success ({})", result.value().to_string());
    return result.value().getResult();
  }
  RTNO_TRACE(m_Logger, "sendData() exit with error");
  return RESULT::ERROR;
}
 
RESULT RTnoProtocol::receiveData(const std::string& portName, uint8_t* data, const uint8_t max_size, uint8_t* size_read, const uint32_t wait_usec, const int retry_count) {
  
  RTNO_TRACE(m_Logger, "receiveData(portName={}, retry={}) called", portName, retry_count);
  int namelen = portName.length();//strlen(portName);
  uint8_t buffer[64];
  buffer[0] = namelen;
  memcpy(buffer+2, portName.c_str(), namelen);
  RTnoPacket packet(COMMAND::RECEIVE_DATA, buffer, (RESULT)0, 2 + namelen);
  m_pTransport->send(packet);
  auto result = waitCommand(COMMAND::RECEIVE_DATA, 20*1000);
  if (result) {
    auto packet = result.value();
    int8_t name_len = packet.getData()[0];
    int8_t data_len = packet.getData()[1];
    *size_read = data_len;
    if (*size_read > max_size) {
      RTNO_ERROR(m_Logger, "receiveData() maximum buffer size exceeded (max={}, received={})", max_size, *size_read);
      RTNO_ERROR(m_Logger, "receiveData() exit with error");
      return packet.getResult();
    }
    memcpy(data, packet.getData()+2+name_len, *size_read);
    RTNO_TRACE(m_Logger, "receiveData() exit with success ({})", result.value().to_string());
    return packet.getResult();
    
  }
  RTNO_ERROR(m_Logger, "receiveData() exit with error");
  return RESULT::ERROR;
}


// void RTnoProtocol::onAddInPort(const RTnoPacket& packet) {
//   m_Profile.appendInPort(parsePortProfile(packet));
// }

// void RTnoProtocol::onAddOutPort(const RTnoPacket& packet) {
//   m_Profile.appendOutPort(parsePortProfile(packet));
// }


#if 0 
uint8_t RTnoProtocol::getRTnoStatus() {
  static const RTnoPacket cmd_packet(GET_STATUS);
  m_pTransport->send(cmd_packet);
  RTnoPacket pac = waitCommand(GET_STATUS, 20*1000);
  return pac.getData()[0];
}

uint8_t RTnoProtocol::getRTnoExecutionContextType() {
  static const RTnoPacket cmd_packet(GET_CONTEXT);
  m_pTransport->send(cmd_packet);
  RTnoPacket pac = waitCommand(GET_CONTEXT, 20*1000);
  return pac.getData()[0];
}

uint8_t RTnoProtocol::activate() {
  static const RTnoPacket cmd_packet(RTNO_ACTIVATE);
  m_pTransport->send(cmd_packet);
  RTnoPacket pac = waitCommand(RTNO_ACTIVATE, 20*1000);
  return pac.getData()[0];
}

uint8_t RTnoProtocol::reset() {
  static const RTnoPacket cmd_packet(RTNO_RESET);
  m_pTransport->send(cmd_packet);
  RTnoPacket pac = waitCommand(RTNO_RESET, 20*1000);
  return pac.getData()[0];
}

uint8_t RTnoProtocol::deactivate() {
  static const RTnoPacket cmd_packet(RTNO_DEACTIVATE);
  m_pTransport->send(cmd_packet);
  RTnoPacket pac = waitCommand(RTNO_DEACTIVATE, 20*1000);
  return pac.getData()[0];
}

uint8_t RTnoProtocol::sendData(const std::string& portName, const uint8_t* data, const uint8_t length) {
  int namelen = portName.length();//strlen(portName);
  uint8_t buffer[64];
  buffer[0] = namelen;
  buffer[1] = length;
  memcpy(buffer+2, portName.c_str(), namelen);
  memcpy(buffer+2+namelen, data, length);
  RTnoPacket packet(SEND_DATA, buffer, 2 + namelen + length);
  m_pTransport->send(packet);
  return 0;
 }


int32_t RTnoProtocol::sendExecuteTrigger(void) {
  static const RTnoPacket cmd_packet(RTNO_EXECUTE);
  return m_pTransport->send(cmd_packet);
}

void RTnoProtocol::receiveData(const uint8_t* data) 
{
  char name_buffer[16];
  memcpy(name_buffer, data+2, data[0]); name_buffer[data[0]] = 0;
  OutPortWrapperBase* outPort = m_pRTObjectWrapper->GetOutPort(name_buffer);
  if(outPort == NULL) {
    throw UnknownOutPortRequestedException();
  } 
	
  outPort->Write((void*)(data + 2 + data[0]), data[1] / outPort->getTypeSizeInArduino());
}

void RTnoProtocol::handleReceivedPacket(const uint32_t wait_usec) {

  if(m_ProxySynchronousExecution) {
    sendExecuteTrigger();
  }

  bool endFlag = false;
  while(!endFlag) {
    if (m_pTransport->isNew()) {
      try {
	RTnoPacket pac = m_pTransport->receive(wait_usec);
	switch(pac.getInterface()) {
	case RECEIVE_DATA:
	  //ReceiveData(packet_buffer);
	  receiveData(pac.getData());
	  break;
	case RTNO_EXECUTE:
	  if(pac.getData()[0] != RTNO_OK) {
	    throw ExecuteFailedException();
	  }
	  endFlag = true;
	  break;
	case SEND_DATA:
	  //this->m_SendBusy = FALSE;
	  break;
	default:
	  std::cout << "Unknown Packet: " << pac.getInterface() << std::endl;
	  endFlag = true;
	  break;
	}
      } catch (TimeOutException& ex) {
	if(!m_ProxySynchronousExecution) {
	  throw ex;
	} else {
	  break;
	}
      }
    }
  }
}

uint8_t RTnoProtocol::initialize()
{
  uint8_t status = getRTnoStatus();
  std::cout << MSGHDR << " - RTno Status=" << (int)status << std::endl;
  int ret;
  switch(status) {
  case STATE_ACTIVE:
    std::cout << MSGHDR << " -- Now Arduino is in ACTIVE STATE. Deactivating..." << std::endl;
    if((ret = deactivate()) != 0) {
      std::cout << MSGHDR << " -- Failed." << std::endl;
      return false;
    }
    std::cout << MSGHDR << " -- OK." << std::endl;
    break;
  case STATE_INACTIVE:
    std::cout << MSGHDR << " -- Now Arduino is in INACTIVE STATE" << std::endl;
    break;
  case STATE_ERROR:
    std::cout << MSGHDR << " -- Now Arduino is in ERROR STATE. Resetting..." << std::endl;
    if((ret = reset()) != 0) {
      std::cout << MSGHDR << " -- Failed." << std::endl;
      return false;
    }
    std::cout << MSGHDR << " -- OK." << std::endl;
    break;
  }
  
  uint8_t contextType = getRTnoExecutionContextType();
  std::cout << MSGHDR <<  "Execution Context Type == " << (int)contextType << std::endl;
  switch(contextType) {
  case ProxySynchronousExecutionContext:
    std::cout << MSGHDR << " - ProxySynchronousExecutionContext is detected." << std::endl;
    std::cout << MSGHDR << " - This Arduino's on_execute is synchronously executed with this PC.\n" << std::endl;
    std::cout << MSGHDR << " - You can change excution ratio by configuring RTnoProxy's one." << std::endl;
    m_ProxySynchronousExecution = true;
    break;
  default:
    std::cout << " - ProxySynchronousExecutionContext is NOT detected." << std::endl;
    std::cout << " - You can change exeecution ratio by implementing Arduino Code\n" << std::endl;
    m_ProxySynchronousExecution = false;
  }
  

  return 0;
}

#endif