#pragma once

#include "system/Logger.h"

#include <chrono>
namespace ch = std::chrono;

namespace timer {

// The simplest timer, just start and get
struct Timer {
	ch::time_point<ch::system_clock> startTime;

	Timer(bool autoStart = false)
	{
		if (autoStart) {
			Start();
		}
	}

	void Start() { Restart(); }
	void Restart() { startTime = ch::system_clock::now(); }

	// If no duration type is passed, takes the parameter of the declaration
	template<typename DurationType = ch::microseconds>
	long long Get() const
	{
		return ch::duration_cast<DurationType>(ch::system_clock::now() - startTime).count();
	}


	auto GetTimeDiff() const { return ch::system_clock::now() - startTime; }
};

// Multitimer, can be used to time multiple passes of the same code and return each time and total.
// This is what the macro use. Declares a utility "Scope" struct that auto reports when it goes out of scope
template<typename ChronoDuration = ch::microseconds>
struct MultiTimer : Timer {
	long long m_total{ 0 };

	MultiTimer(bool autoStart = false)
		: Timer(autoStart)
	{
	}

	void AddReport(const std::string& str)
	{
		auto last = Get<ChronoDuration>();
		m_total += last;
		if constexpr (std::is_same_v<ChronoDuration, ch::microseconds>) {
			LOG_INFO("{0}: {1} micros \tTotal: {2} micros", str, last, m_total);
		}
		else if constexpr (std::is_same_v<ChronoDuration, ch::milliseconds>) {
			LOG_INFO("{0}: {1} ms \tTotal: {2} ms", str, last, m_total);
		}
		else {
			LOG_INFO("{0}: {1} ?? \tTotal: {2} ??", str, last, m_total);
		}
	}

	struct Scope {
		MultiTimer<ChronoDuration>& m_timer;
		std::string m_str;
		Scope(MultiTimer<ChronoDuration>& timer, const std::string& str)
			: m_timer(timer)
			, m_str(str)
		{
			m_timer.Start();
		}

		~Scope() { m_timer.AddReport(m_str); }
		Scope(Scope const&) = default;
		Scope& operator=(Scope const&) = default;
		Scope(Scope&&) = default;
		Scope& operator=(Scope&&) = default;
	};

	Scope StartScope(const std::string& reportString) { return std::move(Scope(*this, reportString)); }
};


template<typename ChronoDuration = ch::microseconds>
class ScopedTimer : Timer {
public:
	std::string m_name;
	ScopedTimer(std::string&& name, bool autoStart = true)
		: Timer(autoStart)
		, m_name(name)
	{
	}


	void Report()
	{
		long long last = Get<ChronoDuration>();
		if constexpr (std::is_same_v<ChronoDuration, ch::microseconds>) { // NOLINT
			LOG_INFO("{0}: {1} micros", m_name, last);
		}
		else if constexpr (std::is_same_v<ChronoDuration, ch::milliseconds>) { // NOLINT
			LOG_INFO("{0}: {1} ms", m_name, last);
		}
		else {
			// Fix report of duration
			LOG_INFO("{0}: {1} ??", m_name, last);
		}
	}

	~ScopedTimer() { Report(); }
	ScopedTimer(ScopedTimer const&) = delete;
	ScopedTimer(ScopedTimer&&) = delete;
	ScopedTimer& operator=(ScopedTimer const&) = delete;
	ScopedTimer& operator=(ScopedTimer&&) = delete;
};


} // namespace timer

#define TIMER_STATIC_SCOPE(Name)                                                                                       \
	static timer::MultiTimer scope_timer;                                                                              \
	auto scope_timer_scope = scope_timer.StartScope(Name);

#define TIMER_STATIC_SCOPE_MS(Name)                                                                                    \
	static timer::MultiTimer<std::chrono::milliseconds> scope_timer_ms;                                                \
	auto scope_timer_scope_ms = scope_timer_ms.StartScope(Name);
