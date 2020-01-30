#include "Test.h"

#include "core/iterable/IterableSafeVector.h"


// Force by copy to ensure std::forward works properly
void InsertByCopy(IterableSafeVector<int32>& safeVec, const int32& value)
{
	safeVec.Emplace(value);
}

TEST("Iterable vector")
{
	IterableSafeVector<int32> safeVec;

	auto& vec = safeVec.vec;

	safeVec.Emplace(1);
	safeVec.Emplace(3);
	InsertByCopy(safeVec, 6);

	auto contains = [&](int32 value) {
		return std::find(vec.begin(), vec.end(), value) != vec.end();
	};

	REQ(vec.size() == 3);
	REQ(contains(1));

	SECT("remove existing")
	{
		safeVec.Remove(1);
		REQ(contains(1) == false);
	}

	SECT("remove non existing")
	{
		safeVec.Remove(2);
		REQ(vec.size() == 3);
	}

	SECT("Add while iterating")
	{
		int32 counter = 0;

		safeVec.BeginSafeRegion();
		for (auto& elem : safeVec.vec) {
			if (elem == 1) {
				safeVec.Emplace(42);
				safeVec.Emplace(43);
			}
			counter++;
		}
		safeVec.EndSafeRegion();

		REQ(counter == 3); // Elements added are not iterated
		REQ(vec.size() == 5);
		REQ(contains(43));
	}

	SECT("Remove while iterating")
	{
		safeVec.Emplace(42);
		safeVec.Emplace(43);

		safeVec.BeginSafeRegion();
		int32 counter = 0;
		for (auto& elem : safeVec.vec) {
			if (elem == 1) {
				safeVec.Remove(1); // remove self
			}
			if (elem == 6) {
				safeVec.Remove(42);
				safeVec.Remove(43);
			}
			counter++;
		}
		safeVec.EndSafeRegion();

		// All elements are iterated even if "removed" during the loop.
		REQ(counter == 5);

		REQ(vec.size() == 2);
		REQ(contains(1) == false);
		REQ(contains(42) == false);
	}

	SECT("Remove all")
	{
		safeVec.Emplace(42);
		safeVec.Emplace(43);

		safeVec.BeginSafeRegion();
		for (auto& elem : safeVec.vec) {
			if (elem == 1) {
				for (auto& elemInner : safeVec.vec) {
					safeVec.Remove(elemInner);
				}
			}
		}
		safeVec.EndSafeRegion();

		REQ(vec.size() == 0);
	}

	SECT("Remove & readd")
	{
		safeVec.Emplace(42);
		safeVec.Emplace(43);

		safeVec.BeginSafeRegion();
		for (auto& elem : safeVec.vec) {
			if (elem == 1) {
				for (auto& elemInner : safeVec.vec) {
					safeVec.Remove(elemInner);
					safeVec.Emplace(elemInner);
				}
			}
		}
		safeVec.EndSafeRegion();

		REQ(vec.size() == 5);
	}
}
