#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_INFO(...) Solana::Logger::get()->info("[{}::{}] {}", FILENAME, __func__, fmt::format(__VA_ARGS__))
#define LOG_WARN(...) Solana::Logger::get()->warn("[{}::{}] {}", FILENAME, __func__, fmt::format(__VA_ARGS__))
#define LOG_ERROR(...) Solana::Logger::get()->error("[{}::{}] {}", FILENAME, __func__, fmt::format(__VA_ARGS__))

namespace Solana
{

    class Logger
    {
    public:
        static std::shared_ptr<spdlog::logger> &get();

    private:
        static std::shared_ptr<spdlog::logger> logger;
        static void init();
    };

}
