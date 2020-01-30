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

		m_uniqueWindows.insert({ hash, elem.constructor(elem.name) });

		return true;
	}
	return false;
}

bool ComponentWindows::CloseUnique(mti::Hash hash)
{
	return m_uniqueWindows.erase(hash) == 1; // WIP: handle delete
}

bool ComponentWindows::ToggleUnique(mti::Hash hash)
{
	if (IsUniqueOpen(hash)) {
		CloseUnique(hash);
		return false;
	}
	return OpenUnique(hash);
}

template<typename DrawFunc>
void ComponentWindows::InternalDraw(DrawFunc&& func)
{
	auto it = begin(m_uniqueWindows);
	auto endIt = end(m_uniqueWindows);

	for (auto& [hash, window] : m_uniqueWindows) {
		func(window.get());
	}

	for (auto& window : m_multiWindows) {
		func(window.get());
	}
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
