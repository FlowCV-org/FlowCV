
#include "FlowLogger.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

FlowLogger::FlowLogger()
{
    spdlog::init_thread_pool(8192, 1);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/log.log", 2, 30);

    std::vector<spdlog::sink_ptr> sinks{console_sink, daily_sink};
    auto logger = std::make_shared<spdlog::async_logger>("flow", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

FlowLogger::Level FlowLogger::getLevel()
{
    switch (spdlog::get_level()) {
        case spdlog::level::level_enum::debug:
            return Level::debug;
        case spdlog::level::level_enum::info:
            return Level::info;
        case spdlog::level::level_enum::warn:
            return Level::warn;
        case spdlog::level::level_enum::off:
            return Level::off;
        default:
            break;
    }
    return Level::debug;
}

void FlowLogger::setLevel(Level l)
{
    auto level = spdlog::level::level_enum::debug;
    switch (l) {
        case Level::debug:
            level = spdlog::level::level_enum::debug;
            break;
        case Level::info:
            level = spdlog::level::level_enum::info;
            break;
        case Level::warn:
            level = spdlog::level::level_enum::warn;
            break;
        case Level::off:
            level = spdlog::level::level_enum::off;
            break;

        default:
            break;
    }
    spdlog::set_level(level);
}
