#pragma once

#include "reflection/TypeId.h"
#include "editor/windows/EdWindow.h"
#include <functional>
#include <unordered_set>
#include <vector>


namespace ed {
// Responsible for handling and aggregating editor windows.
// There are 2 "types" of windows, 'singleton-like' windows where only one can be open at a time and multi windows (eg
// asset editors)
class ComponentWindows {
private:
	void OpenWindow() {}

public:
	struct UniqueWindowEntry {
		std::function<std::unique_ptr<Window>(const std::string&)> constructor;
		size_t hash;
		std::string name;
	};

public:
	std::vector<UniqueWindowEntry> m_entries;
	std::unordered_map<mti::Hash, size_t> m_entiresHash;

	std::unordered_map<mti::Hash, std::unique_ptr<Window>> m_uniqueWindows;
	std::unordered_set<std::unique_ptr<Window>> m_multiWindows;


	// Assumes you won't double add
	template<CONC(WindowClass) T>
	void AddWindowEntry(std::string&& name)
	{
		auto hash = mti::GetHash<T>();
		m_entiresHash.insert({ hash, m_entries.size() });
		m_entries.emplace_back(
			UniqueWindowEntry{ [](const std::string& name) { return std::make_unique<T>(name); }, hash, name });
	};


	[[nodiscard]] bool IsUniqueOpen(mti::Hash hash) const noexcept { return m_uniqueWindows.count(hash); };

	template<CONC(WindowClass) T>
	[[nodiscard]] bool IsUniqueOpen() const noexcept
	{
		return IsUniqueOpen(mti::GetHash<T>());
	};


	// Returns false if hash not registered, true if a window of this type is open (even if not opened right now)
	bool OpenUnique(mti::Hash hash);

	// Returns false if class not registered
	template<CONC(WindowClass) T>
	bool OpenUnique()
	{
		return OpenUnique(mti::GetHash<T>());
	};


	// Returns true if a unique window was closed. false otherwise (not open/not registered)
	bool CloseUnique(mti::Hash hash);

	// Returns true if a unique window was closed. false otherwise (not open/not registered)
	template<CONC(WindowClass) T>
	bool CloseUnique()
	{
		return CloseUnique(mti::GetHash<T>());
	};


	// Returns the new state of the window. (Window may fail to open)
	bool ToggleUnique(mti::Hash hash);

	// Returns the new state of the window. (Window may fail to open)
	template<CONC(WindowClass) T>
	bool ToggleUnique()
	{
		return ToggleUnique(mti::GetHash<T>());
	};
};
} // namespace ed
