#include "pch/pch.h"
#include "system/profiler/Profiler.h"
#include "system/profiler/ProfileScope.h"

void Profiler::Register(ProfileScopeBase* profObj)
{
	auto& p = Get();

	p.m_entries.insert({ profObj->engModule, {} }).first->second.push_back(profObj);
}

void Profiler::BeginFrame()
{
	if (!s_isProfiling) {
		return;
	}

	auto& p = Get();

	for (auto& [cat, vec] : p.m_entries) {
		for (auto& entry : vec) {
			entry->Reset();
		}
	}

	auto now = ch::system_clock::now();

	p.m_lastFrameTime = ch::duration_cast<Precision>(now - p.m_frameBeginTime);
	p.m_frameBeginTime = now;
};
