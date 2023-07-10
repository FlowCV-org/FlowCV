#pragma once

#include <spdlog/spdlog.h>

#include <string>

#ifdef SPDLOG_H
#ifndef LOGGER_INIT
#define LOGGER_INIT FlowLogger();
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#endif
#ifndef LOG_INFO
#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#endif
#ifndef LOG_WARN
#define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#endif
#else
#ifndef LOGGER_INIT
#define LOGGER_INIT (void)0
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG(...) (void)0;
#endif
#ifndef LOG_INFO
#define LOG_INFO(...) (void)0
#endif
#ifndef LOG_WARN
#define LOG_WARN(...) (void)0
#endif
#endif

class FlowLogger
{
  public:
    enum Level : int
    {
        debug = 0,
        info,
        warn,
        off
    };

    FlowLogger();

    static Level getLevel();
    static void setLevel(Level l);
};
