#pragma once

#include "spdlog/logger.h" 
// include for custom formats (override ToString(std::ostream& os) const from engine object)
#include "spdlog/fmt/ostr.h"

// custom formatting
template<class T>
auto operator<<(std::ostream& os, const T& t) -> decltype(t.ToString(os), os)
{
	t.ToString(os);
	
	return os;
}

// custom formatting
template<class T>
auto operator<<(std::ostream& os, T* t) -> decltype(t->ToString(os), os)
{
	if (t)
		t->ToString(os);
	else
		os << "nullptr";

	return os;
}

namespace utl
{
	class Log
	{
	public:
		static void Init(LogLevelTarget level);

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_logger;
	};
}

#define LOGGER_INIT(level) utl::Log::Init(level)

// force log by matching the log level of the logger - except when level is LL_OFF
#define LOG_ANY(...)     utl::Log::GetLogger()->log(utl::Log::GetLogger()->level(), __VA_ARGS__)

#define LOG_TRACE(...)     utl::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)     utl::Log::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)      utl::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)      utl::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)     utl::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...)     utl::Log::GetLogger()->critical(__VA_ARGS__)
#define LOG_ASSERT(...)    do { utl::Log::GetLogger()->critical(__VA_ARGS__); utl::Log::GetLogger()->flush(); std::abort(); } while(0)


#define CLOG_TRACE(condition, ...)     do { if ((condition)) { utl::Log::GetLogger()->trace(__VA_ARGS__) 	; } } while(0)
#define CLOG_DEBUG(condition, ...)     do { if ((condition)) { utl::Log::GetLogger()->debug(__VA_ARGS__) 	; } } while(0)
#define CLOG_INFO(condition, ...)      do { if ((condition)) { utl::Log::GetLogger()->info(__VA_ARGS__)	; } } while(0)
#define CLOG_WARN(condition, ...)      do { if ((condition)) { utl::Log::GetLogger()->warn(__VA_ARGS__) 	; } } while(0)
#define CLOG_ERROR(condition, ...)     do { if ((condition)) { utl::Log::GetLogger()->error(__VA_ARGS__) 	; } } while(0)
#define CLOG_FATAL(condition, ...)     do { if ((condition)) { utl::Log::GetLogger()->critical(__VA_ARGS__); } } while(0)
#define CLOG_ASSERT(condition, ...)    do { if ((condition)) { LOG_ASSERT(__VA_ARGS__); } } while(0)

