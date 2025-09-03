#include "rtno2/Logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <iostream>
#include <memory>
#include <vector>


using namespace ssr;

std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
std::shared_ptr<spdlog::sinks::basic_file_sink_mt>   file_sink;
std::vector<std::shared_ptr<spdlog::sinks::sink>> sink_list;

void ssr::initLogger() {
    if (console_sink) {
        return;
    }
    console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    // console_sink->set_pattern("[] [%^%l%$] %v");

    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("multisink.txt", true);
    file_sink->set_level(spdlog::level::trace);

    sink_list = { file_sink, console_sink };
}

RTnoLogger ssr::getLogger(const std::string& name) {
    initLogger();
    auto logger = spdlog::logger(name, sink_list.begin(), sink_list.end());
    return logger;
}

void ssr::setLogLevel(RTnoLogger* logger, const ssr::LOGLEVEL level) {
    logger->set_level((spdlog::level::level_enum)level);
}
