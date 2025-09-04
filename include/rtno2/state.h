#pragma once

#include <string>
#include <sstream>
#include <cstdint>

namespace ssr::rtno2 {

    // state
    enum class STATE : char {
        CREATED='C',
        INACTIVE='I',
        ACTIVE='A',
        ERR='E',
        NONE='N',
    };

    inline std::string state_to_string(STATE state) {
        switch (state) {
            case STATE::CREATED: return "STATE::CREATED";
            case STATE::INACTIVE: return "STATE::INACTIVE";
            case STATE::ACTIVE: return "STATE::ACTIVE";
            case STATE::ERR: return "STATE::ERROR";
            case STATE::NONE: return "STATE::NONE";
            default: {
                std::stringstream ss;
                ss << "STATE::UNKNOWN(" << (int)state << ")";
                return ss.str();
            }
        }
    }


}