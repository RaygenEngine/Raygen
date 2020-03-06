#pragma once
#include "engine/profiler/ProfilerSetup.h"
#include <chrono>
#include <map>
#include <vector>
#include "core/StringUtl.h"

namespace ch = std::chrono;

struct ProfileScopeBase;

class Profiler {
public:
	using Precision = ch::nanoseconds;
	using Timepoint = ch::time_point<ch::system_clock>;

private:
	Profiler();


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
	Timepoint m_initTime;

	struct ExtendedEntry {
		ProfileScopeBase* scope;
		Timepoint enterTime;
		Timepoint exitTime;
	};


	// Outer vector breaks up entries to batches, prevents super huge copies when inserting.
	// for better results we need to roll our own vector that preallocates
	std::vector<std::vector<ExtendedEntry>> m_sessionRecords;
	std::vector<ExtendedEntry>* m_sessionCurrentVector;

	void BeginFrameSession();

public:
	static inline bool s_isProfiling = ProfilerSetup::c_startsEnabled;
	static inline bool s_shouldStartProfiling = false;
	static inline bool s_shouldEndProfiling = false;

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

	static void BeginProfiling() { s_shouldStartProfiling = true; }
	static void EndProfiling() { s_shouldEndProfiling = true; }

	static void ExportSessionToJson(const fs::path& file);

	static void ResetSession();

	static void ReportExtendedScope(ProfileScopeBase* scope, Timepoint start, Timepoint now)
	{
		Get().m_sessionCurrentVector->push_back({ scope, start, now });
	};

	[[nodiscard]] static Precision GetLastFrameTime() { return Get().m_lastFrameTime; }

	// WARNING: All currently registered entries will be lost until application restart with no way of restoring them
	static void Z_ClearRegistrations() { Get().m_entries.clear(); };
};
