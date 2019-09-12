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
	LOG_DEBUG("Searching for path: \'{}\', recurse: downwards", subPath);

	// if the search path is empty search in the current path
	const auto currPath = searchPath.empty() ? fs::current_path() : searchPath;

	// check self
	{
		auto dataPath = currPath / subPath;

		if (fs::exists(dataPath))
		{
			dataPath = fs::relative(dataPath);
			LOG_DEBUG("found in: \'{}\'", dataPath.string());
			return dataPath;
		}
	}

	for (const auto& entry : fs::recursive_directory_iterator(currPath))
	{
		//LOG_TRACE("searching in: \'{}\'", entry.path().string());
		auto dataPath = entry.path() / subPath;

		if (fs::exists(dataPath))
		{
			dataPath = fs::relative(dataPath);
			LOG_DEBUG("found in: \'{}\'", dataPath.string());
			return dataPath;
		}
	}

	return {};
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

	//auto xsn = SearchPathDownRecursivelyFromCurrent("scenes/test/test.xscn");

	LOG_INFO("Assets root directory: \'{}\'", m_assetsRootPath);

	return true;
}

fs::path PathSystem::SearchAssetPath(const fs::path& asset)
{
	auto ret = SearchPathDownRecursivelyFromPath(asset);

	if (ret.empty())
		LOG_WARN("Could not locate asset, path: {}", asset.string());
	
	return ret;
}
