#pragma once


// Boolean flag that automatically resets to false when read.
// BoolFlags should wherever there are "delayed" event-like notifications and the results need to be processed at a
// specific time
struct BoolFlag {
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

	// TODO: Should we add these:
	// operator bool() { return Access(); }
	// void operator=(bool newValue) { isTrue = newValue; }

protected:
	bool isTrue{ false };
};
