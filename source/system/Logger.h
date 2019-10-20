#pragma once

#include "spdlog/logger.h"
// include for custom formats
#include "spdlog/fmt/ostr.h"
#include <sstream>

namespace utl {
// TODO: should not be in utl namespace
class Log {
public:
	static void Init(LogLevelTarget level);

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }

	static std::stringstream s_editorLogStream;

private:
	static std::shared_ptr<spdlog::logger> s_logger;
};
} // namespace utl

#define LOGGER_INIT(level) utl::Log::Init(level)

#define LOG_REPORT(...) utl::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_TRACE(...)  utl::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)  utl::Log::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)   utl::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)   utl::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)  utl::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...)  utl::Log::GetLogger()->critical(__VA_ARGS__)
#define LOG_ABORT(...)                                                                                                 \
	do {                                                                                                               \
		utl::Log::GetLogger()->critical(__VA_ARGS__);                                                                  \
		utl::Log::GetLogger()->flush();                                                                                \
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
			utl::Log::GetLogger()->trace(__VA_ARGS__);                                                                 \
		}                                                                                                              \
	} while (0)
#define CLOG_DEBUG(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			utl::Log::GetLogger()->debug(__VA_ARGS__);                                                                 \
		}                                                                                                              \
	} while (0)
#define CLOG_INFO(condition, ...)                                                                                      \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			utl::Log::GetLogger()->info(__VA_ARGS__);                                                                  \
		}                                                                                                              \
	} while (0)
#define CLOG_WARN(condition, ...)                                                                                      \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			utl::Log::GetLogger()->warn(__VA_ARGS__);                                                                  \
		}                                                                                                              \
	} while (0)
#define CLOG_ERROR(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			utl::Log::GetLogger()->error(__VA_ARGS__);                                                                 \
		}                                                                                                              \
	} while (0)
#define CLOG_FATAL(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			utl::Log::GetLogger()->critical(__VA_ARGS__);                                                              \
		}                                                                                                              \
	} while (0)
#define CLOG_ABORT(condition, ...)                                                                                     \
	do {                                                                                                               \
		if ((condition)) {                                                                                             \
			LOG_ABORT(__VA_ARGS__);                                                                                    \
		}                                                                                                              \
	} while (0)
