#pragma once

#include <mutex>

#define INVALID_ID     0u
#define FIRST_VALID_ID 1u

namespace utl {
using UID = uint32;

// Context agnostic uuid generator
class UUIDGenerator {
	UID m_currentId = FIRST_VALID_ID;

	UUIDGenerator() = default;

public: // thread safe UUID generator
	static UID GenerateUUID()
	{
		static std::mutex mutex;
		static UUIDGenerator instance;
		mutex.lock();
		const auto v = instance.m_currentId++;

		mutex.unlock();

		return v;
	}

	UUIDGenerator(UUIDGenerator const&) = delete;
	void operator=(UUIDGenerator const&) = delete;
};
} // namespace utl
