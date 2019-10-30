#pragma once

#include <spdlog/logger.h>
// include for custom formats
#include <spdlog/fmt/ostr.h>
#include <sstream>


class Log {
public:
	// NOTE: logging and levels my be discarded by build configuration
	static void Init(LogLevelTarget level);

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }

	static std::stringstream s_editorLogStream;

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};

#define LOGGER_INIT(level) Log::Init(level)


#define _RXN_DO_NOTHING_REQUIRE_SEMICOL()                                                                              \
	do {                                                                                                               \
	} while (0)


//
// BASIC LOGGING
//
#if RXN_WITH_LOG(LOG_BELOW_WARN)
#	define LOG_DEBUG(...) Log::GetLogger()->debug(__VA_ARGS__)
#	define LOG_INFO(...)  Log::GetLogger()->info(__VA_ARGS__)
#else
#	define LOG_DEBUG(...) _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#	define LOG_INFO(...)  _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

#if RXN_WITH_LOG(LOG_ANY)
#	define LOG_WARN(...)   Log::GetLogger()->warn(__VA_ARGS__)
#	define LOG_REPORT(...) Log::GetLogger()->warn(__VA_ARGS__)
#	define LOG_ERROR(...)  Log::GetLogger()->error(__VA_ARGS__)
#else
#	define LOG_WARN(...)   _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#	define LOG_REPORT(...) _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#	define LOG_ERROR(...)  _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

//
// SPECIAL ABORT
//
#if RXN_WITH_LOG(LOG_ANY)
#	define LOG_ABORT(...)                                                                                             \
		do {                                                                                                           \
			Log::GetLogger()->critical(__VA_ARGS__);                                                                   \
			Log::GetLogger()->flush();                                                                                 \
			std::abort();                                                                                              \
		} while (0)
#else
#	define LOG_ABORT(...) _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

//
// CONDITIONAL LOGGING
//
#define CLOG_AT_LEVEL(level, condition, ...)                                                                           \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG##level(__VA_ARGS__);                                                                                   \
		}                                                                                                              \
	} while (0)

#if RXN_WITH_LOG(LOG_BELOW_WARN)
#	define CLOG_DEBUG(condition, ...) CLOG_AT_LEVEL(_DEBUG, condition, __VA_ARGS__)
#	define CLOG_INFO(condition, ...)  CLOG_AT_LEVEL(_INFO, condition, __VA_ARGS__)
#else
#	define CLOG_DEBUG(condition, ...) _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#	define CLOG_INFO(condition, ...)  _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

#if RXN_WITH_LOG(LOG_ANY)
#	define CLOG_WARN(condition, ...)   CLOG_AT_LEVEL(_WARN, condition, __VA_ARGS__)
#	define CLOG_REPORT(condition, ...) CLOG_AT_LEVEL(_REPORT, condition, __VA_ARGS__)
#	define CLOG_ERROR(condition, ...)  CLOG_AT_LEVEL(_ERROR, condition, __VA_ARGS__)
#else
#	define CLOG_WARN(condition, ...)   _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#	define CLOG_REPORT(condition, ...) _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#	define CLOG_ERROR(condition, ...)  _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

#if RXN_WITH_LOG(CLOG_ABORT)
#	define CLOG_ABORT(condition, ...) CLOG_AT_LEVEL(_ABORT, condition, __VA_ARGS__)
#else
#	define CLOG_ABORT(condition, ...) _RXN_DO_NOTHING_REQUIRE_SEMICOL()
#endif
