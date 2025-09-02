#include <iostream>
#include <sstream>
#include <vector>
#include <cstdint>

#include "hal/Serial.h"
#include "rtno2/Transport.h"
#include "rtno2/RTnoProtocol.h"
template<typename T>
T into(const uint8_t* buf, const uint8_t size) {
    return *(T*)buf;
}

// inline int32_t to_long(const uint8_t* buf) {
//     int32_t v = *(int32_t*)buf;
//     return v;
// }

// template<>
// std::int32_t into<std::int32_t>(const uint8_t* buf, const int8_t size) {
//     return to_long(buf);
// }


template<typename T>
inline std::vector<T> into_vec(const uint8_t* buf, const uint8_t size) {
    int length = size / sizeof(T);
    std::vector<T> val;
    for (int i = 0;i < length;i++) {
        val.push_back(into<T>(buf+i*4, 4));
    }
    return val;
}


template<typename T>
inline std::string strjoin(const std::vector<T>& v) {
    std::stringstream ss;
    for(int i = 0;i < v.size();i++) {
        ss << v[i];
        if (i != v.size()-1) {
            ss << ",";
        }
    }
    return ss.str();
}

// inline std::vector<int32_t> to_longs(const uint8_t* buf, const uint8_t size) {
//     int length = size / 4;
//     std::vector<int32_t> val;
//     for (int i = 0;i < length;i++) {
//         val.push_back(to_long(buf+i*4));
//     }
//     return val;
// }


union intermediate_float_uint8 {
    uint8_t uint8_value[4];
    float float_value;
};

inline double to_float(const uint8_t* buf) {
    intermediate_float_uint8 i;
    memcpy(i.uint8_value, buf, 4);
    return i.float_value;
}

union intermediate_double_uint8 {
    uint8_t uint8_value[8];
    double double_value;
};

inline double to_double(const uint8_t* buf) {
    intermediate_double_uint8 i;
    memcpy(i.uint8_value, buf, 8);
    return i.double_value;
}

template<typename T>
std::vector<T> atois(const std::string& str, const char delim, const std::function<T(const std::string&)>& func) {
    std::vector<T> elems;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            std::cout << "elem: " << func(item) << std::endl;
            elems.push_back(func(item));
        }
	}
    return elems;
}

ssr::RTnoState print(ssr::RTnoProtocol& protocol, const std::string& name_str) {
    std::cout << "print(" << name_str << ")" << std::endl;
    std::cout << " - get Profile " << std::endl;
    auto prof = protocol.getRTnoProfile(1000*1000);
    std::cout << " - Prof: " << prof.to_string() << std::endl;

    for (auto port : prof.outPorts()) {
        if (port.getPortName() == name_str) {
            auto type_code = port.getTypeCode();
            if (type_code == TYPECODE_TIMED_BOOLEAN) {
            } else if (type_code == TYPECODE_TIMED_CHAR) {
            } else if (type_code == TYPECODE_TIMED_LONG) {
                uint8_t size;
                uint8_t buffer[64];
                auto state = protocol.receiveData(name_str, buffer, 64, &size);
                int32_t val = into<int32_t>(buffer, size);
                std::cout << val << std::endl;
            } else if (type_code == TYPECODE_TIMED_FLOAT) {
                uint8_t size;
                uint8_t buffer[64];
                auto state = protocol.receiveData(name_str, buffer, 64, &size);
                std::cout << "size :" << (int)size << std::endl;
                float val = to_float(buffer);
                std::cout << val << std::endl;
            } else if (type_code == TYPECODE_TIMED_DOUBLE) {
                uint8_t size;
                uint8_t buffer[64];
                auto state = protocol.receiveData(name_str, buffer, 64, &size);
                std::cout << "size :" << (int)size << std::endl;
                double val = to_double(buffer);
                std::cout << val << std::endl;
            } else if (type_code == TYPECODE_TIMED_LONG_SEQ) {
                uint8_t size;
                uint8_t buffer[64];
                auto state = protocol.receiveData(name_str, buffer, 64, &size);
                std::cout << "received:size :" << (int)size << std::endl;
                auto v = into_vec<int32_t>(buffer, size);
                std::cout << strjoin(v) << std::endl;
            } else if (type_code == TYPECODE_TIMED_FLOAT_SEQ) {
            } else if (type_code == TYPECODE_TIMED_DOUBLE_SEQ) {
            } 
            else {
                return ssr::RTnoState{
                    .status=254,
                };
            }
        }
    }
    return ssr::RTnoState{
        .status=255,
    };
}
ssr::RTnoState inject(ssr::RTnoProtocol& protocol, const std::string& name_str, const std::string& data_str) {
    std::cout << "inject(" << name_str << ", " << data_str << ")" << std::endl;
    std::cout << " - get Profile " << std::endl;
    auto prof = protocol.getRTnoProfile(1000*1000);
    std::cout << " - Prof: " << prof.to_string() << std::endl;

    for (auto port : prof.inPorts()) {
        if (port.getPortName() == name_str) {
            std::cout << " - Found port (" << name_str << ")" << std::endl;
            auto type_code = port.getTypeCode();
            if (type_code == TYPECODE_TIMED_BOOLEAN) {
                uint8_t data = data_str == "true" ? 1 : 0;
                protocol.sendData(name_str, (uint8_t*)(&data), sizeof(uint8_t));
            } else if (type_code == TYPECODE_TIMED_CHAR) {
                char data = data_str.c_str()[0];
                protocol.sendData(name_str, (uint8_t*)(&data), sizeof(char));
            } else if (type_code == TYPECODE_TIMED_LONG) {
                std::cout << " - TimedLong" << std::endl;
                int32_t data = atoi(data_str.c_str());
                protocol.sendData(name_str, (uint8_t*)(&data), sizeof(int32_t));
            } else if (type_code == TYPECODE_TIMED_FLOAT) {
                float data = atof(data_str.c_str());
                protocol.sendData(name_str, (uint8_t*)(&data), sizeof(float));
            } else if (type_code == TYPECODE_TIMED_DOUBLE) {
                double data = atof(data_str.c_str());
                protocol.sendData(name_str, (uint8_t*)(&data), sizeof(double));
            } else if (type_code == TYPECODE_TIMED_LONG_SEQ) {
                auto data = atois<int32_t>(data_str, ',', [](const std::string& s) { return atoi(s.c_str()); });
                protocol.sendData(name_str, (uint8_t*)(&data), data.size() * sizeof(int32_t));
            } else if (type_code == TYPECODE_TIMED_FLOAT_SEQ) {
                auto data = atois<float>(data_str, ',', [](const std::string& s) { return (float)atof(s.c_str()); });
                protocol.sendData(name_str, (uint8_t*)(&data), data.size() * sizeof(float));
            } else if (type_code == TYPECODE_TIMED_DOUBLE_SEQ) {
                auto data = atois<double>(data_str, ',', [](const std::string& s) { return atof(s.c_str()); });
                protocol.sendData(name_str, (uint8_t*)(&data), data.size() * sizeof(double));
            } 
            else {
                return ssr::RTnoState{
                    .status=254,
                };
            }
        }
    }
    return ssr::RTnoState{
        .status=255,
    };
}

int main(const int argc, const char* argv[]) {
    auto filename = argv[1];
    int baudrate = atoi(argv[2]);
    std::string command = argv[3];
    ssr::Serial serial_port(filename, baudrate);
    ssr::Transport transport(&serial_port);
    ssr::RTnoProtocol protocol(&transport);
    if (command == "getprofile") {
            
        auto prof = protocol.getRTnoProfile(1000*1000);
        std::cout << prof.to_string() << std::endl;
    } else if (command == "getstate") {
        auto state = protocol.getRTnoState(1000*1000);
        std::cout << state.to_string() << std::endl;
    } else if (command == "getectype") {
        auto state = protocol.getRTnoExecutionContextType(1000*1000);
        std::cout << state.to_string() << std::endl;
    } else if (command == "activate") {
        auto state = protocol.activateRTno(1000*1000);
        std::cout << state.to_string() << std::endl;
    } else if (command == "deactivate") {
        auto state = protocol.deactivateRTno(1000*1000);
        std::cout << state.to_string() << std::endl;
    } else if (command == "execute") {
        auto state = protocol.executeRTno(1000*1000);
        std::cout << state.to_string() << std::endl;
    } else if (command == "inject") {
        std::string name_str = argv[4];
        std::string data_str = argv[5];
        auto state = inject(protocol, name_str, data_str);
        std::cout << state.to_string() << std::endl;
    } else if (command == "print") {
        std::string name_str = argv[4];
        auto state = print(protocol, name_str);
        std::cout << state.to_string() << std::endl;
    }
    return 0;
}