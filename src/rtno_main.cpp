#include <iostream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>

#include "hal/Serial.h"
#include "rtno2/Transport.h"
#include "rtno2/RTnoProtocol.h"
#include "rtno2/Logger.h"
# include <type_traits>

#include <spdlog/spdlog.h>


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


std::vector<std::string> strsplit(const std::string& str, const char delim) {
    std::vector<std::string> elems;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.push_back(item);
        }
	}
    return elems;
}

template<typename T>
std::vector<T> atois(const std::string& str, const char delim, const std::function<T(const std::string&)>& func) {
    std::vector<T> elems;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.push_back(func(item));
        }
	}
    return elems;
}

template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};


template<typename T, typename = std::enable_if_t<std::is_same_v<T, int>||std::is_same_v<T, float>||std::is_same_v<T, double>>>
std::string result_t_to_string(const ssr::rtno::result_t<T>& result) {
    if (result.result == RESULT::OK) {
        std::stringstream ss;
        ss << "RESULT::OK(" << result.value.value() << ")";
        return ss.str();
    }
    return result_to_string(result.result);
}

template<typename T>
std::string result_t_to_string(const ssr::rtno::result_t<std::vector<T>>& result)  {
    if (result.result == RESULT::OK) {
        std::stringstream ss;
        ss << "RESULT::OK([" << strjoin(result.value.value()) << "])";
        return ss.str();
    }
    return result_to_string(result.result);
}


std::string print(ssr::RTnoLogger& logger, ssr::RTnoProtocol& protocol, const std::string& name_str) {
    RTNO_TRACE(logger, "print({})", name_str);
    auto prof = protocol.getRTnoProfile(1000*1000);
    RTNO_DEBUG(logger, " - getProfile -> {}", prof.to_string());
    for (auto port : prof.outPorts()) {
        if (port.getPortName() == name_str) {
            auto type_code = port.getTypeCode();
            if (type_code == TYPECODE_TIMED_BOOLEAN) {
            } else if (type_code == TYPECODE_TIMED_CHAR) {
            } else if (type_code == TYPECODE_TIMED_LONG) {
                auto val = protocol.receiveAs<int32_t>(name_str);
                return result_t_to_string(val);
            } else if (type_code == TYPECODE_TIMED_FLOAT) {
                auto val = protocol.receiveAs<float>(name_str);
                return result_t_to_string(val);
            } else if (type_code == TYPECODE_TIMED_DOUBLE) {
                auto val = protocol.receiveAs<double>(name_str);
                return result_t_to_string(val);
            } else if (type_code == TYPECODE_TIMED_LONG_SEQ) {
                auto v = protocol.receiveSeqAs<int32_t>(name_str);
                return result_t_to_string(v);
            } else if (type_code == TYPECODE_TIMED_FLOAT_SEQ) {
                auto v = protocol.receiveSeqAs<float>(name_str);
                return result_t_to_string(v);
            } else if (type_code == TYPECODE_TIMED_DOUBLE_SEQ) {
                auto v = protocol.receiveSeqAs<double>(name_str);
                return result_t_to_string(v);
            } 
            else {
                return result_to_string(RESULT::ERROR);
            }
        }
    }
    return result_to_string(RESULT::OUTPORT_NOT_FOUND);
}
RESULT inject(ssr::RTnoLogger& logger, ssr::RTnoProtocol& protocol, const std::string& name_str, const std::string& data_str) {
    RTNO_DEBUG(logger, "inject({}, {})", name_str, data_str);
    auto prof = protocol.getRTnoProfile(1000*1000);
    RTNO_DEBUG(logger, " - getProfile -> {}", prof.to_string());
    for (auto port : prof.inPorts()) {
        if (port.getPortName() == name_str) {
            auto type_code = port.getTypeCode();
            if (type_code == TYPECODE_TIMED_BOOLEAN) {
                uint8_t data = data_str == "true" ? 1 : 0;
                return protocol.sendData(name_str, (uint8_t*)(&data), sizeof(uint8_t));
            } else if (type_code == TYPECODE_TIMED_CHAR) {
                char data = data_str.c_str()[0];
                return protocol.sendData(name_str, (uint8_t*)(&data), sizeof(char));
            } else if (type_code == TYPECODE_TIMED_LONG) {
                int32_t data = atoi(data_str.c_str());
                return protocol.sendData(name_str, (uint8_t*)(&data), sizeof(int32_t));
            } else if (type_code == TYPECODE_TIMED_FLOAT) {
                float data = atof(data_str.c_str());
                return protocol.sendData(name_str, (uint8_t*)(&data), sizeof(float));
            } else if (type_code == TYPECODE_TIMED_DOUBLE) {
                double data = atof(data_str.c_str());
                return protocol.sendData(name_str, (uint8_t*)(&data), sizeof(double));
            } else if (type_code == TYPECODE_TIMED_LONG_SEQ) {
                auto data = atois<int32_t>(data_str, ',', [](const std::string& s) { return atoi(s.c_str()); });
                return protocol.sendData(name_str, (uint8_t*)(&data), data.size() * sizeof(int32_t));
            } else if (type_code == TYPECODE_TIMED_FLOAT_SEQ) {
                auto data = atois<float>(data_str, ',', [](const std::string& s) { return (float)atof(s.c_str()); });
                return protocol.sendData(name_str, (uint8_t*)(&data), data.size() * sizeof(float));
            } else if (type_code == TYPECODE_TIMED_DOUBLE_SEQ) {
                auto data = atois<double>(data_str, ',', [](const std::string& s) { return atof(s.c_str()); });
                return protocol.sendData(name_str, (uint8_t*)(&data), data.size() * sizeof(double));
            } 
            else {
                return RESULT::OK;
            }
        }
    }
    return RESULT::INPORT_NOT_FOUND;
}


int do_main(ssr::RTnoLogger& logger, ssr::RTnoProtocol& protocol, const std::string& command, const std::vector<std::string>& args, const uint32_t wait_usec=1000*1000) {
    if (command == "getprofile") {
        auto prof = protocol.getRTnoProfile(wait_usec);
        std::cout << prof.to_string() << std::endl;
    } else if (command == "getstate") {
        auto state = protocol.getRTnoState(wait_usec);
        std::cout << state.to_string() << std::endl;
    } else if (command == "getectype") {
        auto ectype = protocol.getRTnoExecutionContextType(wait_usec);
        std::cout << ectype.to_string() << std::endl;
    } else if (command == "activate") {
        auto state = protocol.activateRTno(wait_usec);
        std::cout << state.to_string() << std::endl;
    } else if (command == "deactivate") {
        auto state = protocol.deactivateRTno(wait_usec);
        std::cout << state.to_string() << std::endl;
    } else if (command == "execute") {
        auto result = protocol.executeRTno(wait_usec);
        std::cout << result_to_string(result) << std::endl;
    } else if (command == "inject") {
        std::string name_str = args[1];
        std::string data_str = args[2];
        auto state = inject(logger, protocol, name_str, data_str);
        std::cout << result_to_string(state) << std::endl;
    } else if (command == "print") {
        std::string name_str = args[1];
        auto state = print(logger, protocol, name_str);
        std::cout << state << std::endl;
    }
    return 0;
}

int do_interactive(ssr::RTnoLogger& logger, ssr::RTnoProtocol& protocol) {
    while(true) {
        std::cout << "> " << std::ends;
        std::string commandline;
        std::getline(std::cin, commandline);
        auto elems = strsplit(commandline, ' ');
        if (elems.size() == 0) continue;
        // std::cout << strjoin(elems) << std::endl;
        if (elems[0] == "exit") break;
        do_main(logger, protocol, elems[0], elems);
    }
    return 0;
}

int main(const int argc, const char* argv[]) {
    // logger = ssr::getLogger("main");

    ssr::RTnoLogger logger(ssr::getLogger("main"));
    ssr::setLogLevel(&logger, ssr::LOGLEVEL::ERROR);

    std::vector<std::string> args;
    for(int i = 3;i < argc;i++) {
        std::string arg = argv[i];
        args.push_back(arg);
    }
    auto filename = argv[1];
    int baudrate = atoi(argv[2]);
    std::string command = argv[3];
    ssr::Serial serial_port(filename, baudrate);
    ssr::Transport transport(&serial_port);
    ssr::RTnoProtocol protocol(&transport);

    if (command == "interactive") {
        do_interactive(logger, protocol);
        return 0;
    }
    do_main(logger, protocol, command, args);
    return 0;
}