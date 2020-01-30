#pragma once

#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <vector>

// Provides necessary tools for adding/removing elements on a unordered_map while iterating it without errors.
template<typename KeyT, typename ValueT>
struct IterableSafeHashMap {
	using Iterator = typename std::unordered_map<KeyT, ValueT>::iterator;

	std::unordered_map<KeyT, ValueT> map;

	// Stores temporary data to add after loops
	std::unordered_map<KeyT, ValueT> addingCache;

	// Stores temporary iterators to remove after loops
	std::vector<Iterator> erasingCache;

	// Stores iteration state
	bool isIterating{ false };

	// Begins the "safe" region. Call this before your for each loop
	void BeginSafeRegion() { isIterating = true; }

	// Updates the actual "map" with the inserted and deleted items. Call this after your for each loop
	void EndSafeRegion()
	{
		for (auto& removeIt : erasingCache) {
			map.erase(removeIt);
		}

		for (auto& elem : addingCache) {
			map.emplace(std::move(elem));
		}
		addingCache.clear();
		erasingCache.clear();
	}

	// TODO: forward does not work in this context, provide emplace by copy
	auto Emplace(std::pair<KeyT, ValueT>&& elem)
	{
		if (isIterating) {
			return addingCache.emplace(std::forward<std::pair<KeyT, ValueT>&&>(elem)).first;
		}
		return map.emplace(std::forward<std::pair<KeyT, ValueT>&&>(elem)).first;
	}

	// Finds and removes if found.
	// Returns if the element was removed.
	// This function may queue a remove operation and return false. (Happens when a key was found but we are iterating.)
	bool Remove(const KeyT& key) { return Remove(map.find(key)); }

	// Returns if the element was removed.
	// This function may queue a remove operation and return false. (Happens when a key was found but we are iterating.)
	bool Remove(Iterator iterator)
	{
		if (iterator == end(map)) {
			return false;
		}
		if (!isIterating) {
			map.erase(iterator);
			return true;
		}
		erasingCache.push_back(iterator);
		return false;
	}
};
