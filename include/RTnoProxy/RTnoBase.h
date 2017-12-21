#pragma once

#include "Thread.h"

#include "RTnoProfile.h"
#include "RTnoRTObjectWrapper.h"
#include "RTnoProtocol.h"

namespace RTC {
  class DataFlowComponentBase;
};

namespace ssr {


  class RTnoBase : public net::ysuga::Thread {
  private:
    ssr::RTnoProfile m_Profile;
    ssr::RTnoRTObjectWrapper m_RTObjectWrapper;
    ssr::RTnoProtocol m_Protocol;

  public:
    RTnoBase(RTC::DataFlowComponentBase* pRTC, ssr::SerialDevice* pSerial);
    virtual ~RTnoBase();

  public:    
    void initialize();

    bool activate();

    bool deactivate();

    bool execute();

    bool reset();
  };
}
