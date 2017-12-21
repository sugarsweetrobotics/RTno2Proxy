#include "stdafx.h"
#include <coil/Time.h>

#include "Packet.h"


#include "RTnoRTObjectWrapper.h"
#include "RTnoProtocol.h"
#include "InPortWrapper.h"
#include "RTnoBase.h"


//using namespace net::ysuga;
using namespace ssr;

RTnoBase::RTnoBase(RTC::DataFlowComponentBase* pRTC, SerialDevice* pSerial) : 
  m_RTObjectWrapper(pRTC), m_Protocol(&m_RTObjectWrapper, pSerial)
{
}


RTnoBase::~RTnoBase() {
}

static std::string MSGHDR = "[RTnoProxy] ";

void RTnoBase::initialize()
{
  dbg(" - RTnoBase.initialize() called.");
  RTnoProfile profile = m_Protocol.getRTnoProfile(10*1000*1000);
  
  dbg(" -- Parsing RTnoProfile");
  PortList inPorts = profile.inPorts();
  for(PortListIterator it = inPorts.begin();it != inPorts.end();++it) {
    dbg(" --- Adding InPort to ProxyRTC  (name=%s, typeCode=%c)", (*it).getPortName().c_str(), (*it).getTypeCode());
    m_RTObjectWrapper.addInPort((*it));
  }
  PortList outPorts = profile.outPorts();
  for(PortListIterator it = outPorts.begin();it != outPorts.end();++it) {
    dbg(" --- Adding OutPort to ProxyRTC (name=%s, typeCode=%c)", (*it).getPortName().c_str(), (*it).getTypeCode());
    m_RTObjectWrapper.addOutPort((*it));
  }
  dbg(" -- Success");
  m_Protocol.initialize();
  dbg(" -- onInitialized OK");
}



bool RTnoBase::activate()
{
  m_Protocol.activate();
  return true;
}


bool RTnoBase::deactivate()
{
  m_Protocol.deactivate();
  return true;
}


bool RTnoBase::execute()
{
  InPortMap* pInPortMap = m_RTObjectWrapper.GetInPortMap();
  for(InPortMapIterator it = pInPortMap->begin();it != pInPortMap->end();++it) {
    std::string name = (*it).first;
    InPortWrapperBase* inPort = (*it).second;

    if(inPort->isNew()) {
      dbg(" - Reading InPort(name=%s)", name.c_str());
      unsigned char packet_buffer[MAX_PACKET_SIZE];
      int len = inPort->Read();
      inPort->Get(packet_buffer, len);
      dbg(" - Sending Data to RTno Device");
      m_Protocol.sendData(name.c_str(), packet_buffer, len * inPort->getTypeSizeInArduino());
    }
  }
  m_Protocol.handleReceivedPacket(RTNO_INFINITE);
  return true;
}



bool RTnoBase::reset()
{
  return true;
}
