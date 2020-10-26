#pragma once
#include "core/Types.h"

#include <array>
#include <vector>

// Typeless continuous memory span.
// Can be implicitly constructed from vector<T> / array<T>
struct MemorySpan {
private:
	size_t siz;
	byte* ptr;

public:
	byte* data() { return ptr; }
	size_t size() { return siz; }

	byte* begin() { return ptr; }
	byte* end() { return ptr + siz; }


public:
	// Constructors
	template<typename T, size_t Size>
	MemorySpan(std::array<T, Size>& ar)
		: ptr(reinterpret_cast<byte*>(ar.data()))
		, siz(sizeof(T) * ar.size())
	{
	}

	MemorySpan(byte* begin_, size_t size_)
		: ptr(begin_)
		, siz(size_)
	{
	}

	template<typename T>
	MemorySpan(std::vector<T>& vec)
		: ptr(reinterpret_cast<byte*>(vec.data()))
		, siz(sizeof(T) * vec.size())
	{
	}
};
