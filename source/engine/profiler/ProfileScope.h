#pragma once

#include "engine/profiler/Profiler.h"
#include "reflection/TypeId.h"

//
// Notes:
// Storing counters and timers interally at each scope struct should perform better DURING the actual profiling
// part. Resets will be slower but we don't really care as long as we reduce the overhead of the actual profiling.
//

struct ProfileScopeBase {
	const char* file;
	int32 line;
	const char* function;
	size_t hash;
	ProfilerSetup::Module engModule;
	std::string_view frontFacingName;

	size_t hits;
	S_Profiler::Precision sumDuration;

	size_t prevHits;
	S_Profiler::Precision prevSumDuration;


	ProfileScopeBase(const char* file, int32 line, const char* function, ProfilerSetup::Module engModule)
		: file(file)
		, line(line)
		, function(function)
		, engModule(engModule)
		, hash(str::hash(function))
		, hits(0)
		, sumDuration()
	{
		std::string_view trim = " __cdecl ";

		// TODO: make this search portable
		frontFacingName = std::string_view(function);
		auto trimLoc = frontFacingName.find(trim);
		if (trimLoc != std::string::npos) {
			frontFacingName = frontFacingName.substr(trimLoc + trim.size());
		}

		trimLoc = frontFacingName.find_first_of('(');
		if (trimLoc != std::string::npos) {
			frontFacingName = frontFacingName.substr(0, trimLoc);
		}

		frontFacingName = frontFacingName.substr(0, 35);

		Profiler.Register(this);
	}

	void AddExecution(S_Profiler::Precision duration)
	{
		hits++;
		sumDuration += duration;
	}

	void Reset()
	{
		prevHits = hits;
		prevSumDuration = sumDuration;
		hits = 0;
		sumDuration = {};
	}

	template<bool ExtendedScope = ProfilerSetup::c_isDefaultScopeExtended>
	struct Scope {
		static constexpr bool isExtendedScope()
		{
			if constexpr (ProfilerSetup::c_shouldOverrideAllWithDefault) {
				return ProfilerSetup::c_isDefaultScopeExtended;
			}
			return ExtendedScope;
		}

		ProfileScopeBase& owner;
		ch::time_point<ch::system_clock> time;

		Scope(ProfileScopeBase& o)
			: owner(o)
		{
			if (Profiler.m_isProfiling) {
				time = ch::system_clock::now();
			}
		}

		~Scope()
		{
			if (Profiler.m_isProfiling) {
				auto now = ch::system_clock::now();
				owner.AddExecution(now - time);

				if constexpr (isExtendedScope()) {
					Profiler.ReportExtendedScope(&owner, time, now);
				}
			}
		}
	};
};

template<typename ProfilerSetup::Module M, bool = IsEnabled(M)>
struct ProfileScope;

// Disabled profile scope, does nothing
template<ProfilerSetup::Module M>
struct ProfileScope<M, false> {
	ProfileScope(const char*, int32, const char*) {}

	struct Scope {
		Scope(ProfileScope&) {}
	};
};

// Enabled profile scope just inherits the base
template<ProfilerSetup::Module M>
struct ProfileScope<M, true> : ProfileScopeBase {
	ProfileScope(const char* file, int32 line, const char* function)
		: ProfileScopeBase(file, line, function, M)
	{
	}
};


#ifndef __INTELLISENSE__

// TODO: use a single macro and forward arguments
// For parameter use one of ProfilerSetup.h (System, Core, Editor, S_Renderer, World etc)
#	define PROFILE_SCOPE(EngineModule)                                                                                \
		static ProfileScope<::ProfilerSetup::##EngineModule> MACRO_PASTE(z_prof_, __LINE__)(                           \
			__FILE__, __LINE__, MTI_PRETTY_FUNC);                                                                      \
		ProfileScope<::ProfilerSetup::##EngineModule>::Scope MACRO_PASTE(z_prof_sc, __LINE__)(                         \
			MACRO_PASTE(z_prof_, __LINE__));

#	define PROFILE_SCOPE_CHEAP(EngineModule)                                                                          \
		static ProfileScope<ProfilerSetup::##EngineModule> MACRO_PASTE(z_prof_, __LINE__)(                             \
			__FILE__, __LINE__, MTI_PRETTY_FUNC);                                                                      \
		ProfileScope<ProfilerSetup::##EngineModule>::Scope<false> MACRO_PASTE(z_prof_sc, __LINE__)(                    \
			MACRO_PASTE(z_prof_, __LINE__));
#else
#	define PROFILE_SCOPE(EngineModule)
#	define PROFILE_SCOPE_CHEAP(EngineModule)
#endif
