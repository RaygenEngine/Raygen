#pragma once
#include "system/profiler/Profiler.h"

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

	size_t hits;
	Profiler::Precision sumDuration;
	int32 inScope{ false };

	ProfileScopeBase(const char* file, int32 line, const char* function, ProfilerSetup::Module engModule)
		: file(file)
		, line(line)
		, function(function)
		, engModule(engModule)
		, hash(str::StrHash(function))
		, hits(0)
		, sumDuration()
	{
		Profiler::Register(this);
	}

	void AddExecution(Profiler::Precision d)
	{
		hits++;
		sumDuration += d;
	}

	void Reset()
	{
		hits = 0;
		sumDuration = {};
	}

	struct Scope {
		ProfileScopeBase& owner;
		ch::time_point<ch::system_clock> time;

		Scope(ProfileScopeBase& o)
			: owner(o)
		{
			if (Profiler::s_isProfiling) {
				time = ch::system_clock::now();
				owner.inScope++;
			}
		}

		~Scope()
		{
			if (Profiler::s_isProfiling) {
				owner.inScope--;
				owner.AddExecution(ch::duration_cast<Profiler::Precision>(ch::system_clock::now() - time));
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

// For parameter use one of ProfilerSetup.h (System, Core, Editor, Renderer, World etc)
#define PROFILE_SCOPE(EngineModule)                                                                                    \
	static ProfileScope<ProfilerSetup::EngineModule> MACRO_PASTE(z_prof_, __LINE__)(__FILE__, __LINE__, __FUNCSIG__);  \
	ProfileScope<ProfilerSetup::EngineModule>::Scope MACRO_PASTE(z_prof_sc, __LINE__)(MACRO_PASTE(z_prof_, __LINE__));
