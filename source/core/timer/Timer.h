#pragma once

#include <chrono>
namespace ch = std::chrono;


#include "core/logger/Logger.h"

namespace Timer {
template<typename ChronoDuration = ch::microseconds>
class DebugTimer {
	// TODO: use metatemplates when merged with the other branch for specialization check.
	//		static_assert(<ch::duration, ChronoDuration>, "Template parameter must be a std::chrono::duration");
public:
	ch::time_point<ch::system_clock> m_startTime;
	long long m_total{ 0 };
	bool m_stopped{ false };

	DebugTimer() = default;

	DebugTimer(bool autoStart)
	{
		if (autoStart) {
			Start();
		}
	}

	void Start() { m_startTime = ch::system_clock::now(); }

	void StopReport(const std::string& str)
	{
		auto end = ch::system_clock::now();
		long long last = ch::duration_cast<ChronoDuration>(end - m_startTime).count();
		m_total += last;
		if constexpr (std::is_same_v<ChronoDuration, ch::microseconds>) {
			LOG_ANY("{0}: {1} micros \tTotal: {2} micros", str, last, m_total);
		}
		else if constexpr (std::is_same_v<ChronoDuration, ch::milliseconds>) {
			LOG_ANY("{0}: {1} ms \tTotal: {2} ms", str, last, m_total);
		}
		else {
			// Fix report of duration
			LOG_ANY("{0}: {1} ?? \tTotal: {2} ??", str, last, m_total);
		}
	}

	long long Get()
	{
		long long last = ch::duration_cast<ChronoDuration>(ch::system_clock::now() - m_startTime).count();
		m_total += last;
		return last;
	}

	void Stop() { m_stopped = true; }
};


template<typename ChronoDuration = ch::microseconds>
class ScopedTimer {
	// TODO: use metatemplates when merged with the other branch for specialization check.
	//		static_assert(<ch::duration, ChronoDuration>, "Template parameter must be a std::chrono::duration");
public:
	ch::time_point<ch::system_clock> m_startTime;
	std::string m_name;
	ScopedTimer(std::string&& name)
		: m_name(name)
	{
		Restart();
	}

	void Restart() { m_startTime = ch::system_clock::now(); }

	void Report()
	{
		auto end = ch::system_clock::now();
		long long last = ch::duration_cast<ChronoDuration>(end - m_startTime).count();
		if constexpr (std::is_same_v<ChronoDuration, ch::microseconds>) { // NOLINT
			LOG_ANY("{0}: {1} micros", m_name, last);
		}
		else if constexpr (std::is_same_v<ChronoDuration, ch::milliseconds>) { // NOLINT
			LOG_ANY("{0}: {1} ms", m_name, last);
		}
		else {
			// Fix report of duration
			LOG_ANY("{0}: {1} ??", m_name, last);
		}
	}

	~ScopedTimer() { Report(); }
	ScopedTimer(ScopedTimer const&) = default;
	ScopedTimer(ScopedTimer&&) = default;
	ScopedTimer& operator=(ScopedTimer const&) = default;
	ScopedTimer& operator=(ScopedTimer&&) = default;

	long long Get()
	{
		long long last = ch::duration_cast<ChronoDuration>(ch::system_clock::now() - m_startTime).count();
		m_total += last;
		return last;
	}
};

template<typename DurationT = ch::microseconds>
class Scope {
public:
	DebugTimer<DurationT>& m_timer;
	std::string m_str;
	Scope(DebugTimer<DurationT>& timer, const std::string& str)
		: m_timer(timer)
		, m_str(str)
	{
		m_timer.Start();
	}

	~Scope() { m_timer.StopReport(m_str); }
	Scope(Scope const&) = default;
	Scope& operator=(Scope const&) = default;
	Scope(Scope&&) = default;
	Scope& operator=(Scope&&) = default;
};
} // namespace Timer

#define TIMER_STATIC_SCOPE(Name)                                                                                       \
	static Timer::DebugTimer scope_timer;                                                                              \
	Timer::Scope scope_timer_scope(scope_timer, Name)

#define TIMER_STATIC_SCOPE_MS(Name)                                                                                    \
	static Timer::DebugTimer<std::chrono::milliseconds> scope_timer_ms;                                                \
	Timer::Scope<std::chrono::milliseconds> scope_timer_ms_scoe(scope_timer_ms, Name)
