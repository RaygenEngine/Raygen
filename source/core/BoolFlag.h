#pragma once

// Boolean flag that automatically resets to false when read.
// BoolFlags should wherever there are "delayed" event-like notifications and the results need to be processed at a
// specific time
struct BoolFlag {

	constexpr BoolFlag() {}
	constexpr BoolFlag(bool initialValue)
		: isTrue(initialValue)
	{
	}

	// Accesses the value and clears the flag
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

	void Assign(bool newValue) { isTrue = newValue; }

	[[nodiscard]] bool operator==(bool other) { return Access() == other; }
	[[nodiscard]] bool operator!=(bool other) { return Access() != other; }
	[[nodiscard]] bool operator!() { return !Access(); }
	[[nodiscard]] bool operator*() { return Access(); }


protected:
	bool isTrue{ false };
};
