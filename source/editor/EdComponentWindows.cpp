#include "pch.h"
#include "engine/Logger.h"
#include "editor/EdComponentWindows.h"
#include "editor/EdUserSettings.h"

namespace ed {

void ComponentWindows::OpenUnique(mti::Hash hash)
{
	auto it = m_entiresHash.find(hash);

	if (it == m_entiresHash.end()) {
		return;
	}
	if (IsUniqueOpen(hash)) {
		return;
	}

	UpdateSettingsForWindow(m_entries[it->second].name, true);

	// Find out if we need creation, or we have already made this window
	auto itClosed = m_closedUniqueWindows.find(hash);

	if (itClosed != m_closedUniqueWindows.end()) {
		UniqueWindow* winPtr = itClosed->second;
		m_closedUniqueWindows.erase(itClosed);
		m_openUniqueWindows.Emplace({ hash, winPtr });
		return;
	}

	auto& elem = m_entries[it->second];
	m_openUniqueWindows.Emplace({ hash, elem.constructor(elem.name) });
	return;
}

void ComponentWindows::CloseUnique(mti::Hash hash)
{
	auto it = m_openUniqueWindows.map.find(hash);
	if (it == m_openUniqueWindows.map.end()) {
		return;
	}

	UpdateSettingsForWindow(m_entries[m_entiresHash.at(hash)].name, false);

	UniqueWindow* winPtr = it->second;
	m_openUniqueWindows.Remove(it);
	m_closedUniqueWindows.emplace(hash, winPtr);
}

void ComponentWindows::ToggleUnique(mti::Hash hash)
{
	if (IsUniqueOpen(hash)) {
		CloseUnique(hash);
		return;
	}
	OpenUnique(hash);
}

template<typename DrawFunc>
void ComponentWindows::InternalDraw(DrawFunc&& func)
{
	m_openUniqueWindows.BeginSafeRegion();
	for (auto& [hash, window] : m_openUniqueWindows.map) {
		if (!func(window)) {
			CloseUnique(hash);
		}
	}
	m_openUniqueWindows.EndSafeRegion();

	m_multiWindows.BeginSafeRegion();
	for (auto& window : m_multiWindows.vec) {
		func(window.get());
	}
	m_multiWindows.EndSafeRegion();
}

void ComponentWindows::Draw()
{
	m_openUniqueWindows.BeginSafeRegion();
	for (auto& [hash, window] : m_openUniqueWindows.map) {
		if (!window->Z_Draw()) {
			CloseUnique(hash);
		}
	}
	m_openUniqueWindows.EndSafeRegion();

	m_multiWindows.BeginSafeRegion();
	for (auto& window : m_multiWindows.vec) {
		window->Z_Draw();
	}
	m_multiWindows.EndSafeRegion();
}


ComponentWindows::~ComponentWindows()
{
	for (auto& [key, val] : m_openUniqueWindows.map) {
		delete val;
	}

	for (auto& [key, val] : m_closedUniqueWindows) {
		delete val;
	}
}

void ComponentWindows::LoadWindowFromSettings(const std::string& name, mti::Hash hash)
{
	auto& s = ed::GetSettings();

	if (std::find(s.openWindows.begin(), s.openWindows.end(), name) != s.openWindows.end()) {
		OpenUnique(hash);
	}
}

void ComponentWindows::UpdateSettingsForWindow(const std::string& name, bool isOpen)
{
	auto& s = ed::GetSettings();
	if (isOpen) {
		if (std::find(s.openWindows.begin(), s.openWindows.end(), name) == s.openWindows.end()) {
			s.openWindows.push_back(name);
			s.MarkDirty();
		}
	}
	else {
		auto it = std::remove(s.openWindows.begin(), s.openWindows.end(), name);
		if (it != s.openWindows.end()) {
			s.openWindows.erase(it);
			s.MarkDirty();
		}
	}
}

} // namespace ed
