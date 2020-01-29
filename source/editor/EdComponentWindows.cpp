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
} // namespace ed
