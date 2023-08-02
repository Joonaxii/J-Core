#include <J-Core/Log.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace JCore {
    std::shared_ptr<spdlog::logger> Log::_logger;

    void Log::init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/Log.log", 1048576 * 5, 3, true);

        std::vector<spdlog::sink_ptr> sinksCore{ std::make_shared<spdlog::sinks::stderr_color_sink_mt>(), fileSink };

        _logger = std::make_shared<spdlog::logger>("[J-Core]", begin(sinksCore), end(sinksCore));
        spdlog::initialize_logger(_logger);
        _logger->set_level(spdlog::level::trace);
    }
}