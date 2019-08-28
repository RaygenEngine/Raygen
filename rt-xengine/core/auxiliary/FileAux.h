#pragma once

#include <fstream>

namespace Core
{
	template <typename T>
	inline void ReadValueLittleEndianFromFile(std::ifstream& file, T& value)
	{
		file.read(reinterpret_cast<char*>(&value), sizeof(value));
	}

	template <typename T>
	inline void ReadBufferLittleEndianFromFile(std::ifstream& file, T* buffer, uint32 count)
	{
		file.read(reinterpret_cast<char*>(buffer), sizeof(T) * count);
	}
}
