#pragma once

#include <string.h>
#include <sstream>
#include <list>


namespace ssr {
  class PortProfile {
  private:
    uint8_t m_TypeCode;
    std::string m_PortName;
  public:
    PortProfile(uint8_t typeCode, const char* portName) {
      m_TypeCode = typeCode;
      m_PortName = portName;
    }

    PortProfile(const PortProfile &p) {
      m_TypeCode = p.m_TypeCode;
      m_PortName = p.m_PortName;
    }

    std::string to_string() const {
      std::stringstream ss;
      auto typeCode = typecode_to_str(getTypeCode());
      ss << "Port(" << typeCode << ", " << getPortName() << ")";
      return ss.str();
    }
    
    virtual ~PortProfile() {}
    
    uint8_t getTypeCode() const {
      return m_TypeCode;
    }
    
    const std::string& getPortName() const {
      return m_PortName;
    }

    const bool operator==(const PortProfile& p) const {
      return (getPortName() == p.getPortName());
    }
  };
  
  typedef std::list<PortProfile> PortList;
  typedef std::list<PortProfile>::iterator PortListIterator;
  
  
  class RTnoProfile {
  private:
    PortList m_InPorts;
    PortList m_OutPorts;
  public:
    
    RTnoProfile(void) {}

    RTnoProfile(const RTnoProfile& p) {
      m_InPorts = p.m_InPorts;
      m_OutPorts = p.m_OutPorts;
    }
    
    virtual ~RTnoProfile(void) {}

  public:
    std::string to_string() const { 
      std::stringstream ss;
      ss << "RTnoProfile(inports=[";
      for (auto port : m_InPorts) {
        ss << port.to_string() << ",";
      }
      ss << "], outports=[";
      for (auto port : m_OutPorts) {
        ss << port.to_string() << ",";
      }
      ss << "]";
      ss << ")";
      return ss.str();
    }
    
  public:
    
    void appendInPort(const PortProfile& p) {
      m_InPorts.push_back(p);
    }

    void appendOutPort(const PortProfile& p) {
      m_OutPorts.push_back(p);
    }

    
    const PortList& inPorts() {
      return m_InPorts;
    }
    
    const PortList& outPorts() {
      return m_OutPorts;
    }
    
    
    void removeInPort(const char* portName) {
      for(std::list<PortProfile>::iterator it = m_InPorts.begin(); it != m_InPorts.end(); ++it) {
        if((*it).getPortName() == portName) { 
          m_InPorts.remove((*it));
          return;
        }
      }
    }
    
    void removeOutPort(const char* portName) {
      for(std::list<PortProfile>::iterator it = m_OutPorts.begin(); it != m_OutPorts.end(); ++it) {
        if((*it).getPortName() == portName) {
          m_OutPorts.remove((*it));
          return;
        }
      }
    }
    
  };
};
