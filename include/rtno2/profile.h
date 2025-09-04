#pragma once

#include <string.h>
#include <sstream>
#include <list>

#include "type_code.h"

namespace ssr::rtno2 {
  class port_profile_t {
  private:
    TYPECODE typecode_;
    std::string name_;
  public:
    port_profile_t(TYPECODE typeCode, const char* portName) {
      typecode_ = typeCode;
      name_ = portName;
    }

    port_profile_t(const port_profile_t &p) {
      typecode_ = p.typecode_;
      name_ = p.name_;
    }

    std::string to_string() const {
      std::stringstream ss;
      auto typeCode = typecode_to_str(typecode());
      ss << "port_profile_t(" << typeCode << ", " << name() << ")";
      return ss.str();
    }
    
    virtual ~port_profile_t() {}
    
    TYPECODE typecode() const {
      return typecode_;
    }
    
    const std::string& name() const {
      return name_;
    }

    const bool operator==(const port_profile_t& p) const {
      return (name() == p.name());
    }
  };
  
  
  class profile_t {
  public:
    std::list<port_profile_t> inports_;
    std::list<port_profile_t> outports_;
  public:
    
    profile_t(void) {}

    profile_t(const profile_t& p) {
      inports_ = p.inports_;
      outports_ = p.outports_;
    }
    
    virtual ~profile_t(void) {}

  public:
    std::string to_string() const { 
      std::stringstream ss;
      ss << "rtno2::profile_t(inports=[";
      for (auto port : inports_) {
        ss << port.to_string() << ",";
      }
      ss << "], outports=[";
      for (auto port : outports_) {
        ss << port.to_string() << ",";
      }
      ss << "]";
      ss << ")";
      return ss.str();
    }
    
  public:
    
    void append_in_port(const port_profile_t& p) {
      inports_.push_back(p);
    }

    void append_out_port(const port_profile_t& p) {
      outports_.push_back(p);
    }
    
    void remove_in_port(const char* portName) {
      for(std::list<port_profile_t>::iterator it = inports_.begin(); it != inports_.end(); ++it) {
        if((*it).name() == portName) { 
          inports_.remove((*it));
          return;
        }
      }
    }
    
    void remove_out_port(const char* portName) {
      for(std::list<port_profile_t>::iterator it = outports_.begin(); it != outports_.end(); ++it) {
        if((*it).name() == portName) {
          outports_.remove((*it));
          return;
        }
      }
    }
    
  };
};
