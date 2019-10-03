#include "pch.h"

#include "asset/PathSystem.h"

#include <filesystem>

namespace fs = std::filesystem;

fs::path PathSystem::SearchPathUpRecursivelyFromCurrent(const fs::path& subPath)
{
	LOG_DEBUG("Searching for path: \'{}\', recurse: upwards", subPath);
	for (auto currPath = fs::current_path(); ; currPath = currPath.parent_path())
	{
		LOG_TRACE("searching in: \'{}\', {}", currPath.string(), currPath.parent_path().string());
		for (const auto& entry : fs::directory_iterator(currPath))
		{
			if (!entry.is_directory())
			{
				continue;
			}
			auto dataPath = entry.path() / subPath;

			if (fs::exists(dataPath))
			{
				LOG_DEBUG("found in: \'{}\'", dataPath.string());
				return dataPath;
			}
		}

		// reached end, check C 
		if (currPath == currPath.parent_path())
		{
			auto dataPath = currPath / subPath;

			if (fs::exists(dataPath))
			{
				LOG_DEBUG("found in: \'{}\'", currPath.string());
				return dataPath;
			}
			break;
		}
	}

	return {};
}

fs::path PathSystem::SearchPathDownRecursivelyFromPath(const fs::path& subPath, const fs::path& searchPath)
{
	//TIMER_STATIC_SCOPE("Resolve recursive path search");
	
	LOG_DEBUG("Searching for path: \'{}\', recurse: downwards", subPath);

	// if the search path is empty search in the current path
	const auto currPath = searchPath.empty() ? fs::current_path() : searchPath;

	auto fileToFind = utl::ToLower(subPath.string());

	for (const auto& entry : fs::recursive_directory_iterator(currPath))
	{
		// Case sensitive compare.
		if (utl::ToLower(entry.path().filename().string()) == fileToFind)
		{
			return entry;
		}
	}

	return {};
}

void PathSystem::CacheAssetFilenames()
{
	Timer::DebugTimer<std::chrono::milliseconds> timer(true);

	for (const auto& entry : fs::recursive_directory_iterator(fs::current_path()))
	{
		if (entry.is_directory())
		{
			continue;
		}
		auto relative = fs::relative(entry);
		auto filename = relative.filename().string();
		auto relativeStr = relative.string();
		if (!m_fileCache.count(filename))
		{
			m_fileCache.insert({ utl::force_move(filename), relativeStr });
			m_fileCache.insert({ relativeStr, utl::force_move(relativeStr) });
		}
	}
	LOG_INFO("Cached {} asset filenames in {} ms.", m_fileCache.size(), timer.Get());
}

bool PathSystem::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	LOG_INFO("Initializing Paths System");

	auto appParentPath = fs::path(applicationPath);

	if(!appParentPath.has_parent_path())
	{
		LOG_FATAL("Couldn't retrieve application's parent directory!");
		return false; 
	}

	appParentPath = appParentPath.parent_path();

	std::error_code error;
	fs::current_path(appParentPath, error);
	if (error)
	{
		LOG_FATAL("Couldn't set work directory!");
		return false;
	}

	m_assetsRootPath = SearchPathUpRecursivelyFromCurrent(dataDirectoryName);

	fs::current_path(m_assetsRootPath, error);
	if (error)
	{
		LOG_ERROR("Couldn't locate assets root directory!");
		return false;
	}
	LOG_INFO("Assets root directory: \'{}\'", m_assetsRootPath);

	CacheAssetFilenames();
	return true;
}

fs::path PathSystem::SearchAssetPath(const fs::path& asset)
{
	auto it = m_fileCache.find(asset.string());
	if (it != m_fileCache.end())
	{
		return fs::path(it->second);
	}

	if (fs::exists(asset))
	{
		return asset;
	}

	auto ret = SearchPathDownRecursivelyFromPath(asset);

	// TODO: cache this result

	if (ret.empty())
	{
		if (asset.has_parent_path())
		{
			ret = SearchAssetPath(asset.filename());
		}
	}
	
	return ret;
}

void PathSystem::GenerateFileListOfType(const fs::path& extension, std::set<std::string>& outFiles) const
{
	for (auto p : m_fileCache)
	{
		auto thisExtension = fs::path(p.first).extension();
		if (thisExtension == extension)
		{
			outFiles.emplace(fs::path(p.first).filename().string());
		}
	}
}
