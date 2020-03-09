#pragma once

// Boolean flag that automatically resets to false when read.
// BoolFlags should wherever there are "delayed" event-like notifications and the results need to be processed at a
// specific time
// TODO: make non default constructor, add constexpr
struct BoolFlag {

	BoolFlag() {}
	BoolFlag(bool initialValue)
		: isTrue(initialValue)
	{
	}

	bool Access()
	{
		if (!isTrue) {
			return false;
		}
		isTrue = false;
		return true;
	}

	void Set() { isTrue = true; }
	void Clear() { isTrue = false; }

	bool operator==(bool other) { return Access() == other; }
	bool operator!=(bool other) { return Access() != other; }
	bool operator!() { return !Access(); }
	bool operator*() { return Access(); }

protected:
	bool isTrue{ false };
};
