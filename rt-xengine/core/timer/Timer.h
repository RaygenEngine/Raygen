#pragma once

#include <chrono>

// TODO: make or use an actual timer
#define TIMING
#ifdef TIMING
#define INIT_TIMER auto start___ = std::chrono::high_resolution_clock::now()
#define START_TIMER  start___ = std::chrono::high_resolution_clock::now()
#define STOP_TIMER(name)  RT_XENGINE_LOG_AT_LOWEST_LEVEL("{0}: {1} ms", name,  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start___).count())
#define STOP_TIMER_MICRO(name)  RT_XENGINE_LOG_AT_LOWEST_LEVEL("{0}: {1} micros", name,  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-start___).count())
#else
#define TIME_FUNCTION(func, ...)
#define INIT_TIMER
#define START_TIMER
#define STOP_TIMER(name)
#endif


#include "core/logger/Logger.h"
namespace Timer
{
	namespace ch = std::chrono;
	class Debug
	{
	public:
		ch::time_point<ch::system_clock> m_startTime;
		long long m_total{ 0 };

		void Start()
		{
			m_startTime = ch::system_clock::now();
		}

		void Stop(const std::string& str)
		{
			long long last = ch::duration_cast<ch::microseconds>(ch::system_clock::now() - m_startTime).count();
			m_total += last;
			RT_XENGINE_LOG_AT_LOWEST_LEVEL("{0}: {1} micros \tTotal: {2} micros", str, last, m_total);
		}
	};

	class Scope
	{
	public:
		Debug& m_timer;
		std::string m_str;
		Scope(Debug& timer, const std::string& str)
			:m_timer(timer)
			, m_str(str)
		{
			m_timer.Start();
		}

		~Scope() 
		{
			m_timer.Stop(m_str);
		}
	};
}

#define TIMER_STATIC_SCOPE(Name) \
static Timer::Debug scope_timer____LINE__; Timer::Scope scope_timer____LINE__scope(scope_timer____LINE__, Name)