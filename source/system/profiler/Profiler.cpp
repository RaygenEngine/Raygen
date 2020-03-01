#include "pch/pch.h"
#include "system/profiler/Profiler.h"
#include "system/profiler/ProfileScope.h"
#include "system/Logger.h"
#include <fstream>

Profiler::Profiler()
{
	m_sessionRecords.resize(1);
	m_sessionCurrentVector = &m_sessionRecords[0];
	m_initTime = ch::system_clock::now();
}

void Profiler::Register(ProfileScopeBase* profObj)
{
	auto& p = Get();

	p.m_entries.insert({ profObj->engModule, {} }).first->second.push_back(profObj);
}

void Profiler::BeginFrame()
{
	if (s_shouldStartProfiling) {
		s_shouldStartProfiling = false;
		s_isProfiling = true;
	}

	if (s_shouldEndProfiling) {
		s_shouldEndProfiling = false;
		s_isProfiling = false;
	}

	if (!s_isProfiling) {
		return;
	}

	auto& p = Get();
	p.BeginFrameSession();

	for (auto& [cat, vec] : p.m_entries) {
		for (auto& entry : vec) {
			entry->Reset();
		}
	}

	auto now = ch::system_clock::now();

	p.m_lastFrameTime = ch::duration_cast<Precision>(now - p.m_frameBeginTime);
	p.m_frameBeginTime = now;
}

void Profiler::ExportSessionToJson(const fs::path& file)
{
	auto& records = Get().m_sessionRecords;

	std::ofstream output(file);

	size_t entries = 0;

	output << "[";
	for (auto& vec : records) {
		for (auto& record : vec) {
			[[likely]] if (entries++ > 0) { output << ","; }

			long long enter = ch::duration_cast<ch::microseconds>(record.enterTime - Get().m_initTime).count();
			long long duration = ch::duration_cast<ch::microseconds>(record.exitTime - record.enterTime).count();

			output << "{";
			output << "\"cat\":\"function\",";
			output << "\"dur\":" << duration << ',';
			output << "\"name\":\"" << record.scope->frontFacingName << "\",";
			output << "\"ph\":\"X\",";
			output << "\"pid\":0,";
			// TODO: THREAD:
			// output << "\"tid\":" << result.ThreadID << ",";
			output << "\"ts\":" << enter;
			output << "}";
		}
	}
	output << "]";
	LOG_INFO("Exported Profiler Session: {} entries at: {}", entries, file.generic_string());
}

void Profiler::ResetSession()
{
	auto& p = Get();

	p.m_sessionRecords.clear();
	p.m_sessionRecords.resize(1);
	p.m_sessionCurrentVector = &p.m_sessionRecords[0];
};

void Profiler::BeginFrameSession()
{
	constexpr size_t minElementsPerBatch = 2048 - 512;

	if (m_sessionCurrentVector->size() > minElementsPerBatch) {
		m_sessionCurrentVector = &m_sessionRecords.emplace_back();
	}
}
