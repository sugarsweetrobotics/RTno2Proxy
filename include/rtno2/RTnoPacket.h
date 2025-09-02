

#pragma once
#include <stdint.h>
#include <string.h> // memcpy

#include <iostream>

#include "Packet.h"

#define PACKET_HEADER_SIZE 2
#define PACKET_RECEIVE_HEADER_SIZE 3
#define PACKET_SENDER_INFO_LENGTH 4

namespace ssr {

  class RTnoPacket {
  private:
    uint8_t *m_pData;
    RESULT m_result;
  public:
    COMMAND getInterface() const {return (COMMAND)m_pData[0];}
    RESULT getResult() const {return (RESULT)getData()[0];}
    uint8_t getDataLength() const { return m_pData[1];}
    //uint8_t *getSenderInfo() {return m_pData + PACKET_HEADER_SIZE;}
    uint8_t getPacketLength() const { return PACKET_HEADER_SIZE + m_pData[1]; }
    const uint8_t *getData() const {return m_pData + PACKET_HEADER_SIZE;}
    const uint8_t *serialize() const {return m_pData;}
    uint8_t getSum() const {uint8_t sum = (uint8_t)m_result; for(uint32_t i = 0;i < getPacketLength();i++) sum+=m_pData[i]; return sum;}

  private:
    void initialize(const uint8_t interFace, const uint8_t* data = NULL, const uint8_t size = 0) {
      delete m_pData;
      //m_pData = new uint8_t[size + PACKET_HEADER_SIZE + PACKET_SENDER_INFO_LENGTH];
      m_pData = new uint8_t[size + PACKET_HEADER_SIZE];
      m_pData[0] = interFace;
      m_pData[1] = size;
      //memcpy(m_pData + PACKET_HEADER_SIZE, sender, PACKET_SENDER_INFO_LENGTH);
      if(size > 0) {
	//memcpy(m_pData + PACKET_HEADER_SIZE + PACKET_SENDER_INFO_LENGTH,
	memcpy(m_pData + PACKET_HEADER_SIZE,
	       data, size);
      }
    }
  public:
    void dump();
  public:
  RTnoPacket(const uint8_t* p, const uint8_t size) : m_pData(NULL) {
      //uint8_t size = m_pData[1] + PACKET_HEADER_SIZE + PACKET_SENDER_INFO_LENGTH;
      m_pData = new uint8_t[size];
      memcpy(m_pData, p, size);
    }
    
  RTnoPacket(const uint8_t interFace,  const uint8_t* data = NULL, const uint8_t size = 0) :
    m_pData(NULL){
      initialize(interFace, data, size);
    }
  // RTnoPacket(const COMMAND interFace,  const uint8_t* data = NULL, const uint8_t size = 0) :
  //   m_pData(NULL){
  //     initialize((uint8_t)interFace, data, size);
  //   }

  RTnoPacket(const COMMAND command,  const uint8_t* data = NULL, const RESULT result=(RESULT)0, const uint8_t size = 0) :
    m_pData(NULL), m_result(result) {
      initialize((uint8_t)command, data, size);
    }
    
  RTnoPacket(const RTnoPacket& p) : m_pData(NULL) {
      initialize((uint8_t)p.getInterface(),
		 //p.getSenderInfo(), 
		 p.getData(),
		 p.getDataLength());
    }


  public:

    void operator=(const RTnoPacket& p) {
      initialize((uint8_t)p.getInterface(), 
		 //p.getSenderInfo(), 
		 p.getData(), 
		 p.getDataLength());
    }
      
    ~RTnoPacket() {
      delete m_pData;
    }
    
  public:
    
    
  };
}
