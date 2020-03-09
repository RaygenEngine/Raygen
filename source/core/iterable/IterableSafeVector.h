#pragma once

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

// Provides necessary tools for adding/removing elements on a vector while iterating it without errors.
// Order of elements is not preserved
template<typename T>
struct IterableSafeVector {
	using Iterator = typename std::vector<T>::iterator;
	std::vector<T> vec;

	// Stores temporary data to add after loops
	std::vector<T> addingCache;

	// Stores temporary indicies to remove after loops
	std::set<size_t, std::greater<size_t>> erasingCacheIndicies;

	// Stores iteration state
	bool isIterating{ false };

	// Begins the "safe" region. Call this before your for each loop
	void BeginSafeRegion() { isIterating = true; }

	// Updates the actual "vec" with the inserted and deleted items. Call this after your for each loop
	void EndSafeRegion()
	{
		int64 swapPos = static_cast<int64>(vec.size() - 1);
		for (auto& removalPos : erasingCacheIndicies) {
			std::swap(vec[removalPos], vec[swapPos]);
			swapPos--;
		}
		vec.erase(vec.begin() + (swapPos + 1), vec.end());
		erasingCacheIndicies.clear();

		vec.insert(end(vec), make_move_iterator(addingCache.begin()), make_move_iterator(addingCache.end()));
		addingCache.clear();
	}

	template<typename Tt>
	T& Emplace(Tt&& value)
	{
		if (isIterating) {
			return addingCache.emplace_back(std::forward<Tt>(value));
		}
		return vec.emplace_back(std::forward<Tt>(value));
	}

	// Finds and removes if found
	// Double remove while iterating is NOT handled
	void Remove(const T& value) { Remove(std::find(begin(vec), end(vec), value)); }


	// Double remove while iterating is NOT handled
	void Remove(Iterator iterator)
	{
		if (iterator == end(vec)) {
			return;
		}
		if (!isIterating) {
			vec.erase(iterator);
			return;
		}
		erasingCacheIndicies.insert(iterator - vec.begin());
	}
};
