#pragma once

#define INVALID_ID 0u

namespace utl {
using UID = uint32;

// Context agnostic uuid generator
class UUIDGenerator {
	UID m_currentId{ 1u };

	UUIDGenerator() = default;

public: // thread safe UUID generator
	static UID GenerateUUID()
	{
		static UUIDGenerator instance;
		const auto v = instance.m_currentId++;
		return v;
	}

	~UUIDGenerator() = default;

	UUIDGenerator(UUIDGenerator const&) = delete;
	UUIDGenerator(UUIDGenerator&&) = delete;

	UUIDGenerator& operator=(UUIDGenerator const&) = delete;
	UUIDGenerator& operator=(UUIDGenerator&&) = delete;
};
} // namespace utl
