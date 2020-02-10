#pragma once

#include "reflection/TypeId.h"
#include "editor/windows/EdWindow.h"
#include "core/iterable/IterableSafeVector.h"
#include "core/iterable/IterableSafeHashMap.h"
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
		std::function<Window*(const std::string&)> constructor;
		size_t hash;
		std::string name;
	};

public:
	std::vector<UniqueWindowEntry> m_entries;
	std::unordered_map<mti::Hash, size_t> m_entiresHash;

	IterableSafeHashMap<mti::Hash, Window*> m_openUniqueWindows;
	std::unordered_map<mti::Hash, Window*> m_closedUniqueWindows;

	IterableSafeVector<std::unique_ptr<Window>> m_multiWindows;


	template<CONC(UniqueWindowClass) T>
	void AddWindowEntry(const std::string& name)
	{
		auto hash = mti::GetHash<T>();
		if (m_entiresHash.insert({ hash, m_entries.size() }).second) {
			m_entries.emplace_back(
				UniqueWindowEntry{ [](const std::string& name) { return new T(name); }, hash, name });
		}
	};


	[[nodiscard]] bool IsUniqueOpen(mti::Hash hash) const noexcept { return m_openUniqueWindows.map.count(hash); };

	template<CONC(UniqueWindowClass) T>
	[[nodiscard]] bool IsUniqueOpen() const noexcept
	{
		return IsUniqueOpen(mti::GetHash<T>());
	};


	void OpenUnique(mti::Hash hash);

	template<CONC(UniqueWindowClass) T>
	void OpenUnique()
	{
		OpenUnique(mti::GetHash<T>());
	};


	void CloseUnique(mti::Hash hash);

	template<CONC(UniqueWindowClass) T>
	void CloseUnique()
	{
		CloseUnique(mti::GetHash<T>());
	};


	void ToggleUnique(mti::Hash hash);

	template<CONC(UniqueWindowClass) T>
	void ToggleUnique()
	{
		ToggleUnique(mti::GetHash<T>());
	};

	void Draw();

	~ComponentWindows();

private:
	template<typename DrawFunc>
	void InternalDraw(DrawFunc&& func);
};
} // namespace ed