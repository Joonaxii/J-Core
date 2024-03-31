#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#define STR(X) #X

namespace JCore {
    class Log {
    public:
        static void init();
        static std::shared_ptr<spdlog::logger>& getLogger() { return _logger; };
    private:
        static std::shared_ptr<spdlog::logger> _logger;
    };
}

#define JCORE_DEBUGBREAK() __debugbreak()

#define JCORE_EXPAND_MACRO(x) x
#define JCORE_STRINGIFY_MACRO(x) #x

//Core Log Macros
#define JCORE_ERROR(...) ::JCore::Log::getLogger()->error(__VA_ARGS__)
#define JCORE_WARN(...)  ::JCore::Log::getLogger()->warn(__VA_ARGS__)
#define JCORE_INFO(...)  ::JCore::Log::getLogger()->info(__VA_ARGS__)
#define JCORE_TRACE(...) ::JCore::Log::getLogger()->trace(__VA_ARGS__)

#define JCORE_INTERNAL_ASSERT_IMPL(check, msg, ...) { if(!(check)) { JCORE_ERROR(msg, __VA_ARGS__); JCORE_DEBUGBREAK(); } }
#define JCORE_INTERNAL_ASSERT_WITH_MSG(check, ...) JCORE_INTERNAL_ASSERT_IMPL(check, "Assertion failed: {0}", __VA_ARGS__)
#define JCORE_INTERNAL_ASSERT_NO_MSG(check) JCORE_INTERNAL_ASSERT_IMPL(check, "Assertion '{0}' failed at {1}:{2}", JCORE_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define JCORE_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define JCORE_INTERNAL_ASSERT_GET_MACRO(...) JCORE_EXPAND_MACRO( JCORE_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, JCORE_INTERNAL_ASSERT_WITH_MSG, JCORE_INTERNAL_ASSERT_NO_MSG) )

#define JCORE_ASSERT(...) JCORE_EXPAND_MACRO( JCORE_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(__VA_ARGS__) )
