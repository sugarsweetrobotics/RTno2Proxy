// -*- C++ -*-
/*!
 * @file  RTnoProxy.cpp
 * @brief Proxy RTC with arduino compatible device and RTnoProxy library
 * @date 11/11/2010
 * @author Yuki Suga (ysuga@ysuga.net)
 * $Id$
 */

#include "stdafx.h"
#include "RTnoProxy.h"
#include "EtherTcp.h"
#include "Serial.h"


#include "RTnoProtocol.h"


// Module specification
// <rtc-template block="module_spec">



static const char* rtno_spec[] =
  {
    "implementation_id", "RTnoProxy",
    "type_name",         "RTnoProxy",
    "description",       "Proxy RTC with arduino compatible device and RTnoProxy library",
    "version",           "1.0.0",
    "vendor",            "ysuga.net",
    "category",          "Test",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "0",
    "language",          "C++",
    "lang_type",         "compile",
#ifdef WIN32
	"conf.default.comport", "\\\\.\\COM10",
	"conf.default.baudrate", "57600", 
#else
    //"conf.default.comport", "/dev/tty2",
#endif
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
RTnoProxy::RTnoProxy(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager)
  //    m_doubleInIn("in", m_doubleIn), m_doubleOutOut("out", m_doubleOut)
    // </rtc-template>
{
  m_pRTno = NULL;
  m_pSerialDevice = NULL;
}

/*!
 * @brief destructor
 */
RTnoProxy::~RTnoProxy()
{
}




static std::string MSGHDR = "[RTnoProxy] ";

RTC::ReturnCode_t RTnoProxy::onInitialize()
{
  bindParameter("connectionType", m_connectionType, "serial");
  bindParameter("ipAddress", m_ipAddress, "192.168.1.2");
  bindParameter("portNumber", m_portNumber, "23");
  bindParameter("comport", m_comport, "/dev/tty0");
  bindParameter("baudrate", m_baudrate, "19200");
  
  
  updateParameters("default");
  dbg(" - Configuration Values");
  dbg(" -- conf.default.connectionType:%s", m_connectionType.c_str());
  try {
    if (m_connectionType == "serial") {
      dbg(" - Serial Port Connection is selected..");
      dbg(" -- conf.default.comport :%s", m_comport.c_str());
      dbg(" -- conf.default.baudrate:%d", m_baudrate);
      m_pSerialDevice = new ssr::Serial(m_comport.c_str(), m_baudrate);
    } else {
      dbg(" - TCP Port Connection is required.");
      dbg(" -- conf.default.portNumber:%d", m_portNumber);
      dbg(" -- conf.default.ipAddress :%s", m_ipAddress.c_str());
      m_pSerialDevice = new ssr::EtherTcp(m_ipAddress.c_str(), m_portNumber);
    }
  } catch (std::exception& e) {
    dbg(" - Failed: %s", e.what());
    dbg(" - INSTRUCTION:");
    dbg(" -- This error is usucally caused by the misconfiguration of RTnoProxy.");
    dbg(" -- Whan using RTnoProxy, default configuration modification is recommended.");
    dbg(" -- Configuration Values can be modified with 'rtc.conf' file.");
    dbg(" -- The file is usually placed in the 'current directory (program launched directory)'");
    dbg(" -- Or, you can specify your own rtc.conf file with -f option.");
    dbg(" -- Ex., $ RTnoProxyComp -f my_rtc_conf.conf");
    dbg(" -- In rtc.conf file, RTC specific configuration file can be specified like...");
    dbg(" -- Embed.RTnoProxy.config_file: RTnoProxy.conf");
    dbg(" -- The above line specifies that the RTnoProxy is configured by RTnoProxy.conf file.");
    dbg(" -- In the RTnoProxy.conf file, the configuration values are set like...");
    dbg(" -- conf.default.connectionType: seiral");
    dbg(" -- Please check the attached rtc.conf and RTnoProxy.conf files.");
    dbg(" -- And please modify as your RTno fits to your environment.");
    dbg(" -- ");
    dbg(" -- But you can also enjoy RTno with dynamic configuration and launch.");
    dbg(" -- Launch RT System Editor and configure the configuration of RTnoProxy and activate it.");
    dbg(" -- After the activation, ports and values will be added if the configuration is proper.");
    return RTC::RTC_OK;
  }

  
  dbg(" - Starting RTnoProxy...");
  m_pRTno = new ssr::RTnoBase(this, m_pSerialDevice);

  coil::TimeValue interval(0, 1000*1000);
  dbg(" - Waiting for Startup the arduino...");
  dbg(" - 3........");  coil::sleep(interval);
  dbg(" - 2......");  coil::sleep(interval);
  dbg(" - 1....");  coil::sleep(interval);
  dbg(" - Go!");
  dbg("");
  dbg(" - Starting up onInitialize sequence.");

  try {
    m_pRTno->initialize();
  } catch (std::exception& ex) {
    dbg(" - ERROR: %s", ex.what());
    dbg(" -- RTnoProxy failed to initialize RTno device connection.");
    dbg(" -- ");
    dbg(" -- But you can also enjoy RTno with dynamic configuration and launch.");
    dbg(" -- Launch RT System Editor and configure the configuration of RTnoProxy and activate it.");
    dbg(" -- After the activation, ports and values will be added if the configuration is proper.");
    delete m_pSerialDevice;
    delete m_pRTno;
    return RTC::RTC_OK;
  }
  dbg(" - RTnoProxy is successfully initialized");
	
  return RTC::RTC_OK;
}

RTC::ReturnCode_t RTnoProxy::onFinalize()
{
  delete m_pSerialDevice;
  delete m_pRTno;
  m_pRTno = NULL;
  m_pSerialDevice = NULL;
  return RTC::RTC_OK;
}


/*
RTC::ReturnCode_t RTnoProxy::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t RTnoProxy::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t RTnoProxy::onActivated(RTC::UniqueId ec_id)
{
  try {
    dbg(" - Activating RTno");
    m_pRTno->activate();
  } catch (std::exception& e) {
    dbg(" - Activating RTno failed: %s", e.what());
    return RTC::RTC_ERROR;
  }

  dbg(" - Successfully activated.");
  return RTC::RTC_OK;
}


RTC::ReturnCode_t RTnoProxy::onDeactivated(RTC::UniqueId ec_id)
{
  try {
    dbg(" - Deactivating RTno");
    m_pRTno->deactivate();
  } catch (std::exception& e) {
    dbg(" - Deactivagin RTno failed: %s", e.what());
    return RTC::RTC_ERROR;
  }
  
  dbg(" - Successfully deactivated.");
  return RTC::RTC_OK;
}


RTC::ReturnCode_t RTnoProxy::onExecute(RTC::UniqueId ec_id)
{
  try {
    m_pRTno->execute();
  } catch (std::exception& e) {
    dbg(" - onExecute failed: %s", e.what());
    return RTC::RTC_ERROR;
  }
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t RTnoProxy::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t RTnoProxy::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}


RTC::ReturnCode_t RTnoProxy::onReset(RTC::UniqueId ec_id)
{
  try {
    m_pRTno->reset();
  } catch (std::exception& e) {
    dbg(" - onReset failed: %s", e.what());
    return RTC::RTC_ERROR;
  }
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t RTnoProxy::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t RTnoProxy::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void RTnoProxyInit(RTC::Manager* manager)
  {
    coil::Properties profile(rtno_spec);
    manager->registerFactory(profile,
                             RTC::Create<RTnoProxy>,
                             RTC::Delete<RTnoProxy>);
  }
  
};


