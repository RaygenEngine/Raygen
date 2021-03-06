#include "Profiler.h"

#include "engine/profiler/ProfileScope.h"

#include <fstream>

Profiler_::Profiler_()
{
	m_sessionRecords.resize(1);
	m_sessionCurrentVector = &m_sessionRecords[0];
	m_initTime = ch::system_clock::now();
}

void Profiler_::Register(ProfileScopeBase* profObj)
{
	m_entries.insert({ profObj->engModule, {} }).first->second.push_back(profObj);
}

void Profiler_::BeginFrame()
{
	if (*m_shouldStartProfiling) {
		m_isProfiling = true;
	}

	if (*m_shouldEndProfiling) {
		m_isProfiling = false;
	}

	if (!m_isProfiling) {
		return;
	}

	BeginFrameSession();

	for (auto& [cat, vec] : m_entries) {
		for (auto& entry : vec) {
			entry->Reset();
		}
	}

	auto now = ch::system_clock::now();

	m_lastFrameTime = ch::duration_cast<Precision>(now - m_frameBeginTime);
	m_frameBeginTime = now;
}

void Profiler_::ExportSessionToJson(const fs::path& file)
{
	auto& records = m_sessionRecords;

	std::ofstream output(file);

	size_t entries = 0;

	output << "[";
	for (auto& vec : records) {
		for (auto& record : vec) {
			[[likely]] if (entries++ > 0) { output << ","; }

			long long enter = ch::duration_cast<ch::microseconds>(record.enterTime - m_initTime).count();
			long long duration = ch::duration_cast<ch::microseconds>(record.exitTime - record.enterTime).count();

			output << "{";
			output << "\"cat\":\"function\",";
			output << "\"dur\":" << duration << ',';
			output << "\"name\":\"" << record.scope->frontFacingName << "\",";
			output << "\"ph\":\"X\",";
			output << "\"pid\":0,";
			// output << "\"tid\":" << result.ThreadID << ",";
			output << "\"ts\":" << enter;
			output << "}";
		}
	}
	output << "]";
	LOG_INFO("Exported Profiler Session: {} entries at: {}", entries, file.generic_string());
}

void Profiler_::ResetSession()
{
	m_sessionRecords.clear();
	m_sessionRecords.resize(1);
	m_sessionCurrentVector = &m_sessionRecords[0];
};

void Profiler_::BeginFrameSession()
{
	constexpr size_t minElementsPerBatch = 2048 - 512;

	if (m_sessionCurrentVector->size() > minElementsPerBatch) {
		m_sessionCurrentVector = &m_sessionRecords.emplace_back();
	}
}
