#pragma once

#include "spdlog/logger.h"
// include for custom formats
#include "spdlog/fmt/ostr.h"
#include <sstream>


class Log {
public:
	static void Init(LogLevelTarget level);

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }

	static std::stringstream s_editorLogStream;

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};

#define LOGGER_INIT(level) Log::Init(level)

#define LOG_REPORT(...) Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_TRACE(...)  Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)  Log::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)   Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)   Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)  Log::GetLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...)  Log::GetLogger()->critical(__VA_ARGS__)
#define LOG_ABORT(...)                                                                                                 \
	do {                                                                                                               \
		LOG_FATAL(__VA_ARGS__);                                                                                        \
		Log::GetLogger()->flush();                                                                                     \
		std::abort();                                                                                                  \
	} while (0)

#define CLOG_REPORT(condition, ...)                                                                                    \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_REPORT(__VA_ARGS__);                                                                                   \
		}                                                                                                              \
	} while (0)
#define CLOG_TRACE(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_TRACE(__VA_ARGS__);                                                                                    \
		}                                                                                                              \
	} while (0)
#define CLOG_DEBUG(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_DEBUG(__VA_ARGS__);                                                                                    \
		}                                                                                                              \
	} while (0)
#define CLOG_INFO(condition, ...)                                                                                      \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_INFO(__VA_ARGS__);                                                                                     \
		}                                                                                                              \
	} while (0)
#define CLOG_WARN(condition, ...)                                                                                      \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_WARN(__VA_ARGS__);                                                                                     \
		}                                                                                                              \
	} while (0)
#define CLOG_ERROR(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_ERROR(__VA_ARGS__);                                                                                    \
		}                                                                                                              \
	} while (0)
#define CLOG_FATAL(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_FATAL(__VA_ARGS__);                                                                                    \
		}                                                                                                              \
	} while (0)
#define CLOG_ABORT(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_ABORT(__VA_ARGS__);                                                                                    \
		}                                                                                                              \
	} while (0)
