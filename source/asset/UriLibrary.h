#pragma once
// The central library for the engines Uri convention.

#include <filesystem>
namespace fs = std::filesystem;

namespace uri
{
	constexpr char CpuChar = '#';

	inline bool IsCpuPath(const fs::path& path)
	{
		if (!path.has_filename())
		{
			return false;
		}
		return path.filename().string()[0] == CpuChar;
	}

	inline bool HasDirectory(const fs::path& path)
	{
		if (IsCpuPath(path))
		{
			return path.parent_path().has_parent_path();
		}
		return path.has_parent_path();
	}

	// Returns the short version that will get found by our path system, preserving the uri information
	inline fs::path RemovePathDirectory(const fs::path& path)
	{
		if (IsCpuPath(path))
		{
			return path.parent_path().filename() / path.filename();
		}
		return path.filename();
	}
}
