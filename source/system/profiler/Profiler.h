#pragma once
#include "system/profiler/ProfilerSetup.h"
#include <chrono>
#include <map>
#include <vector>
#include "core/StringAux.h"

namespace ch = std::chrono;

struct ProfileScopeBase;

class Profiler {
public:
	using Precision = ch::nanoseconds;

private:
	Profiler() = default;

protected:
	Profiler(const Profiler&) = delete;
	Profiler(Profiler&&) = delete;
	Profiler operator=(const Profiler&) = delete;
	Profiler operator=(Profiler&&) = delete;


	[[nodiscard]] static Profiler& Get()
	{
		static Profiler instance;
		return instance;
	}

	std::map<ProfilerSetup::Module, std::vector<ProfileScopeBase*>> m_entries;


	ch::time_point<ch::system_clock> m_frameBeginTime{};
	Precision m_lastFrameTime;

public:
	static inline bool s_isProfiling = ProfilerSetup::c_startsEnabled;

	static void Register(ProfileScopeBase* profObj);

	static void BeginFrame();

	// Don't modify even if its not const
	[[nodiscard]] static decltype(m_entries)& GetAll() { return Get().m_entries; };

	[[nodiscard]] static std::vector<ProfileScopeBase*>* GetModule(ProfilerSetup::Module m)
	{
		auto& entries = Get().m_entries;
		auto it = entries.find(m);
		if (it != entries.end()) {
			return &it->second;
		}
		return nullptr;
	};


	[[nodiscard]] constexpr static bool IsModuleEnabled(ProfilerSetup::Module m) { return IsEnabled(m); }

	static void BeginProfiling() { s_isProfiling = true; }

	static void EndProfiling() { s_isProfiling = false; }

	[[nodiscard]] static Precision GetLastFrameTime() { return Get().m_lastFrameTime; }

	// WARNING: All currently registered entries will be lost until application restart with no way of restoring them
	static void Z_ClearRegistrations() { Get().m_entries.clear(); };
};