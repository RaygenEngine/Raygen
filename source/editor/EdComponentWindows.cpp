#include "pch/pch.h"
#include "editor/EdComponentWindows.h"

namespace ed {

bool ComponentWindows::OpenUnique(mti::Hash hash)
{
	auto it = m_entiresHash.find(hash);

	if (it != m_entiresHash.end()) {
		if (IsUniqueOpen(hash)) {
			return true;
		}

		auto& elem = m_entries[it->second];

		m_uniqueWindows.Emplace({ hash, elem.constructor(elem.name) });

		return true;
	}
	return false;
}

bool ComponentWindows::CloseUnique(mti::Hash hash)
{
	return m_uniqueWindows.Remove(hash) == 1; // WIP: handle delete
}

bool ComponentWindows::ToggleUnique(mti::Hash hash)
{
	if (IsUniqueOpen(hash)) {
		return !CloseUnique(hash);
	}
	return OpenUnique(hash);
}

template<typename DrawFunc>
void ComponentWindows::InternalDraw(DrawFunc&& func)
{
	m_uniqueWindows.BeginSafeRegion();
	for (auto& [hash, window] : m_uniqueWindows.map) {
		func(window.get());
	}
	m_uniqueWindows.EndSafeRegion();

	m_multiWindows.BeginSafeRegion();
	for (auto& window : m_multiWindows.vec) {
		func(window.get());
	}
	m_multiWindows.EndSafeRegion();
}

void ComponentWindows::Draw()
{
	InternalDraw([](Window* w) { w->Z_Draw(); });
}

void ComponentWindows::ZTest_Draw()
{
	InternalDraw([](Window* w) { w->OnDraw(); });
}


} // namespace ed
