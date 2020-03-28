#pragma once
#include "core/StringUtl.h"
#include "engine/profiler/ProfilerSetup.h"

#include <chrono>
#include <map>
#include <vector>

namespace ch = std::chrono;

struct ProfileScopeBase;

inline class Profiler_ {
public:
	using Precision = ch::nanoseconds;
	using Timepoint = ch::time_point<ch::system_clock>;

	Profiler_();

	Profiler_(const Profiler_&) = delete;
	Profiler_(Profiler_&&) = delete;
	Profiler_ operator=(const Profiler_&) = delete;
	Profiler_ operator=(Profiler_&&) = delete;

protected:
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
	bool m_isProfiling{ ProfilerSetup::c_startsEnabled };
	BoolFlag m_shouldStartProfiling{ false };
	BoolFlag m_shouldEndProfiling{ false };

	void Register(ProfileScopeBase* profObj);

	void BeginFrame();

	// Don't modify even if its not const
	[[nodiscard]] auto& GetEntries() { return m_entries; };

	[[nodiscard]] std::vector<ProfileScopeBase*>* GetModule(ProfilerSetup::Module m)
	{
		auto it = m_entries.find(m);
		if (it != m_entries.end()) {
			return &it->second;
		}
		return nullptr;
	};


	[[nodiscard]] constexpr static bool IsModuleEnabled(ProfilerSetup::Module m) { return IsEnabled(m); }

	void BeginProfiling() { m_shouldStartProfiling = true; }
	void EndProfiling() { m_shouldEndProfiling = true; }

	void ExportSessionToJson(const fs::path& file);

	void ResetSession();

	void ReportExtendedScope(ProfileScopeBase* scope, Timepoint start, Timepoint now)
	{
		m_sessionCurrentVector->push_back({ scope, start, now });
	};

	[[nodiscard]] Precision GetLastFrameTime() { return m_lastFrameTime; }

	// WARNING: All currently registered entries will be lost until application restart with no way of restoring them
	void Z_ClearRegistrations() { m_entries.clear(); };
} Profiler{};
