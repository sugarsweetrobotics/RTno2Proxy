#pragma once

#include <spdlog/spdlog.h>

namespace ssr {

    using RTnoLogger = spdlog::logger;

    enum class LOGLEVEL {
        TRACE = spdlog::level::trace,
        DEBUG = spdlog::level::debug,
        INFO = spdlog::level::info,
        WARN = spdlog::level::warn,
        ERROR = spdlog::level::err,
        CRITICAL = spdlog::level::critical,
        NONE = spdlog::level::off,
    };
    void initLogger();
    RTnoLogger getLogger(const std::string& name);
    void setLogLevel(RTnoLogger* logger, const ssr::LOGLEVEL level);

}

#define RTNO_TRACE(logger, __VA_ARGS__...) (logger).trace(__VA_ARGS__)
#define RTNO_DEBUG(logger, __VA_ARGS__...) (logger).debug(__VA_ARGS__)
#define RTNO_INFO(logger, __VA_ARGS__...) (logger).info(__VA_ARGS__)
#define RTNO_WARN(logger, __VA_ARGS__...) (logger).warn(__VA_ARGS__)
#define RTNO_ERROR(logger, __VA_ARGS__...) (logger).error(__VA_ARGS__)
#define RTNO_CRITICAL(logger, __VA_ARGS__...) (logger).critical(__VA_ARGS__)