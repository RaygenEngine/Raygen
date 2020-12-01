#pragma once

// include for custom formats
#include <spdlog/fmt/ostr.h>
#include <sstream>

enum class LogLevel
{
	Trace = 0,
	Debug,
	Info,
	Warn,
	Error,
	Critical,
	Off
};

// Enables logging through dll. (Intented to hold all logs and then be replayed when out of the dll)
// Dll warning: uses containers/strings
// Also, this does not get to use macros, so formatting will ALWAYS occur even if log level is disabled through macros
//
// Flushes automatically when going out of scope
struct LogTransactions {
	std::vector<std::pair<LogLevel, std::string>> logs;

	void Debug(std::string&& str) { logs.emplace_back(LogLevel::Debug, std::move(str)); }
	void Info(std::string&& str) { logs.emplace_back(LogLevel::Info, std::move(str)); }
	void Warn(std::string&& str) { logs.emplace_back(LogLevel::Warn, std::move(str)); }
	void Report(std::string&& str) { logs.emplace_back(LogLevel::Warn, std::move(str)); }
	void Error(std::string&& str) { logs.emplace_back(LogLevel::Error, std::move(str)); }
	void Critical(std::string&& str) { logs.emplace_back(LogLevel::Critical, std::move(str)); }

	// RAII flush (will be skipped automatically if replay is called manually)
	~LogTransactions();
};

inline class Log_ {
public:
	// NOTE: logging and levels my be discarded by build configuration
	void Init(LogLevel level);

	std::stringstream s_editorLogStream;
	void EarlyInit();


	void Debug(const std::string& str);
	void Info(const std::string& str);
	void Warn(const std::string& str);
	void Error(const std::string& str);
	void Critical(const std::string& str);

	void Flush();

	void Replay(LogTransactions& trans);

private:
	void BasicSetup();
} Log;


#define _RGN_DO_NOTHING_REQUIRE_SEMICOL()                                                                              \
	do {                                                                                                               \
	} while (0)


//
// BASIC LOGGING
//
#if RGN_WITH_LOG(LOG_BELOW_WARN)
#	define LOG_DEBUG(...) Log.Debug(fmt::format(__VA_ARGS__))
#	define LOG_INFO(...)  Log.Info(fmt::format(__VA_ARGS__))
#else
#	define LOG_DEBUG(...) _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#	define LOG_INFO(...)  _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

#if RGN_WITH_LOG(LOG_ANY)
#	define LOG_WARN(...)   Log.Warn(fmt::format(__VA_ARGS__))
#	define LOG_REPORT(...) Log.Warn(fmt::format(__VA_ARGS__))
#	define LOG_ERROR(...)  Log.Error(fmt::format(__VA_ARGS__))
#else
#	define LOG_WARN(...)   _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#	define LOG_REPORT(...) _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#	define LOG_ERROR(...)  _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

//
// SPECIAL ABORT
//
#if RGN_WITH_LOG(LOG_ANY)
#	define LOG_ABORT(...)                                                                                             \
		do {                                                                                                           \
			Log.Critical(fmt::format(__VA_ARGS__));                                                                    \
			Log.Flush();                                                                                               \
			std::abort();                                                                                              \
		} while (0)
#else
#	define LOG_ABORT(...) _RGN_DO_NOTHING_REQUIRE_SEMICOL()
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

#if RGN_WITH_LOG(LOG_BELOW_WARN)
#	define CLOG_DEBUG(condition, ...) CLOG_AT_LEVEL(_DEBUG, condition, __VA_ARGS__)
#	define CLOG_INFO(condition, ...)  CLOG_AT_LEVEL(_INFO, condition, __VA_ARGS__)
#else
#	define CLOG_DEBUG(condition, ...) _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#	define CLOG_INFO(condition, ...)  _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

#if RGN_WITH_LOG(LOG_ANY)
#	define CLOG_WARN(condition, ...)   CLOG_AT_LEVEL(_WARN, condition, __VA_ARGS__)
#	define CLOG_REPORT(condition, ...) CLOG_AT_LEVEL(_REPORT, condition, __VA_ARGS__)
#	define CLOG_ERROR(condition, ...)  CLOG_AT_LEVEL(_ERROR, condition, __VA_ARGS__)
#else
#	define CLOG_WARN(condition, ...)   _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#	define CLOG_REPORT(condition, ...) _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#	define CLOG_ERROR(condition, ...)  _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#endif

#if RGN_WITH_LOG(CLOG_ABORT)
#	define CLOG_ABORT(condition, ...) CLOG_AT_LEVEL(_ABORT, condition, __VA_ARGS__)
#else
#	define CLOG_ABORT(condition, ...) _RGN_DO_NOTHING_REQUIRE_SEMICOL()
#endif
