#ifndef FILEAUX_H
#define FILEAUX_H

#include <fstream>
#include <vector>

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

	struct XMDFileData
	{
		std::vector<char> buffer;
		uint32 pos = 0;
	};

	inline void ReadNullTerminatedStringFromBuffer(XMDFileData& data, std::string& value)
	{
		value = "";
		char c;
		while((c = data.buffer[data.pos++])) { value += c; }
	}

	template <typename T>
	inline void ReadValueLittleEndianFromBuffer(XMDFileData& data, T& value)
	{
		memcpy(&value, &data.buffer[data.pos], sizeof(value));
		data.pos += sizeof(value);
	}

	template <typename T>
	inline void ReadBufferLittleEndianFromBuffer(XMDFileData& data, T* buffer, uint32 count)
	{
		memcpy(buffer, &data.buffer[data.pos], sizeof(T) * count);
		data.pos += sizeof(T) * count;
	}

	template <typename T>
	inline void ReadVectorLittleEndianFromBuffer(XMDFileData& data, std::vector<T>& buffer)
	{
		memcpy(buffer.data(), &data.buffer[data.pos], sizeof(T) * buffer.size());
		data.pos += sizeof(T) * static_cast<uint32>(buffer.size());
	}
}

#endif // FILEAUX_H
