#include "Test.h"
#include "core/iterable/IterableSafeHashMap.h"

#include <string>

TEST("Basic")
{
	IterableSafeHashMap<std::string, int32> safeMap;

	auto& map = safeMap.map;

	safeMap.Emplace({ "foo", 3 });
	safeMap.Emplace({ "Sup", 4 });
	safeMap.Emplace({ "bar", 8 });

	REQ(map.size() == 3);

	SECT("remove existing")
	{
		safeMap.Remove("Sup");
		REQ(map.contains("Sup") == false);
	}

	SECT("remove non existing")
	{
		safeMap.Remove("Sup2");
		REQ(map.size() == 3);
	}

	SECT("Add while iterating")
	{
		safeMap.BeginSafeRegion();
		int32 counter = 0;
		for (auto& [key, val] : safeMap.map) {
			if (key == "foo") {
				safeMap.Emplace({ "something", 42 });
				safeMap.Emplace({ "more", 43 });
			}
			counter++;
		}
		safeMap.EndSafeRegion();
		REQ(counter == 3); // Elements added are not iterated
		REQ(map.size() == 5);
		REQ(map["more"] == 43);
	}

	SECT("Remove while iterating")
	{
		safeMap.Emplace({ "cat", 42 });
		safeMap.Emplace({ "dog", 43 });

		safeMap.BeginSafeRegion();
		int32 counter = 0;
		for (auto& [key, val] : safeMap.map) {
			if (key == "foo") {
				safeMap.Remove("foo"); // remove self
			}
			if (key == "bar") {
				safeMap.Remove("Sup");
				safeMap.Remove("cat");
			}
			counter++;
		}
		safeMap.EndSafeRegion();

		// All elements are iterated even if "removed" during the loop.
		REQ(counter == 5);

		REQ(map.size() == 2);
		REQ(map.contains("cat") == false);
		REQ(map.contains("dog") == true);
		REQ(map.contains("foo") == false);
	}
}

TEST("Extended")
{
	IterableSafeHashMap<int32, int32> safeMap;
	auto& map = safeMap.map;

	for (int32 i = 0; i < 5; i++) {
		safeMap.Emplace({ i, i });
	}
	REQ(map.size() == 5);

	SECT("Remove All")
	{
		safeMap.BeginSafeRegion();
		for (auto& [key, value] : safeMap.map) {
			if (key == 3) {
				for (auto& [keyInner, valueInner] : safeMap.map) {
					safeMap.Remove(keyInner);
				}
			}
		}
		safeMap.EndSafeRegion();
		REQ(map.size() == 0);
	}

	SECT("Remove All & Readd")
	{
		safeMap.BeginSafeRegion();
		for (auto& [key, value] : safeMap.map) {
			if (key == 3) {
				for (auto& [keyInner, valueInner] : safeMap.map) {
					safeMap.Remove(keyInner);
				}
				for (auto& [keyInner, valueInner] : safeMap.map) {
					safeMap.Emplace({ keyInner, valueInner });
				}
			}
		}
		safeMap.EndSafeRegion();
		REQ(map.size() == 5);
	}
}
