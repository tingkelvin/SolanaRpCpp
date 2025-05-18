#include "Solana/Logger.hpp"

namespace Solana
{

    std::shared_ptr<spdlog::logger> Logger::logger = nullptr;

    void Logger::init()
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/app.log", true);

        console_sink->set_level(spdlog::level::debug);
        file_sink->set_level(spdlog::level::debug);

        logger = std::make_shared<spdlog::logger>("SolanaLogger", spdlog::sinks_init_list{console_sink, file_sink});
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        spdlog::register_logger(logger);
    }

    std::shared_ptr<spdlog::logger> &Logger::get()
    {
        if (!logger)
        {
            init();
        }
        return logger;
    }

}
