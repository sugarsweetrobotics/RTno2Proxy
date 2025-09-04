#include "rtno2/logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <iostream>
#include <memory>
#include <vector>


using namespace ssr::rtno2;

std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
std::shared_ptr<spdlog::sinks::basic_file_sink_mt>   file_sink;
std::vector<std::shared_ptr<spdlog::sinks::sink>> sink_list;

void ssr::rtno2::init_logger() {
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

ssr::rtno2::logger_t ssr::rtno2::get_logger(const std::string& name) {
    init_logger();
    auto logger = spdlog::logger(name, sink_list.begin(), sink_list.end());
    return logger;
}

void ssr::rtno2::set_log_level(logger_t* logger, const ssr::rtno2::LOGLEVEL level) {
    logger->set_level((spdlog::level::level_enum)level);
}
