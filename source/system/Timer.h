#pragma once

#include "system/Logger.h"

#include <chrono>
namespace ch = std::chrono;

// TODO: refactor duplicate timers.
namespace timer {
template<typename ChronoDuration = ch::microseconds>
class DebugTimer {

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
			LOG_REPORT("{0}: {1} micros \tTotal: {2} micros", str, last, m_total);
		}
		else if constexpr (std::is_same_v<ChronoDuration, ch::milliseconds>) {
			LOG_REPORT("{0}: {1} ms \tTotal: {2} ms", str, last, m_total);
		}
		else {
			// Fix report of duration
			LOG_REPORT("{0}: {1} ?? \tTotal: {2} ??", str, last, m_total);
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
			LOG_REPORT("{0}: {1} micros", m_name, last);
		}
		else if constexpr (std::is_same_v<ChronoDuration, ch::milliseconds>) { // NOLINT
			LOG_REPORT("{0}: {1} ms", m_name, last);
		}
		else {
			// Fix report of duration
			LOG_REPORT("{0}: {1} ??", m_name, last);
		}
	}

	~ScopedTimer() { Report(); }
	ScopedTimer(ScopedTimer const&) = delete;
	ScopedTimer(ScopedTimer&&) = delete;
	ScopedTimer& operator=(ScopedTimer const&) = delete;
	ScopedTimer& operator=(ScopedTimer&&) = delete;

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
	Scope(Scope const&) = delete;
	Scope& operator=(Scope const&) = delete;
	Scope(Scope&&) = delete;
	Scope& operator=(Scope&&) = delete;
};
} // namespace timer

#define TIMER_STATIC_SCOPE(Name)                                                                                       \
	static timer::DebugTimer scope_timer;                                                                              \
	timer::Scope scope_timer_scope(scope_timer, Name)

#define TIMER_STATIC_SCOPE_MS(Name)                                                                                    \
	static timer::DebugTimer<std::chrono::milliseconds> scope_timer_ms;                                                \
	timer::Scope<std::chrono::milliseconds> scope_timer_ms_scoe(scope_timer_ms, Name)
