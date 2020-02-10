#pragma once
#include "editor/windows/EdWindow.h"
#include "system/profiler/ProfilerSetup.h"
#include "asset/AssetManager.h"

namespace ed {
struct AssetFileEntry {
	AssetFileEntry() {}
	AssetFileEntry(const std::string& p, PodEntry* e = nullptr)
		: subpath(p)
		, entry(e)
	{
	}

	AssetFileEntry(std::string&& p, PodEntry* e = nullptr)
		: subpath(std::move(p))
		, entry(e)
	{
	}

	AssetFileEntry(std::string_view p, PodEntry* e = nullptr)
		: subpath(std::string(p))
		, entry(e)
	{
	}


	// Nullptr if folder
	std::string subpath{ "/" };
	PodEntry* entry{ nullptr };
	bool containsDirectories{ false };
	std::unordered_map<std::string, std::unique_ptr<AssetFileEntry>> children;
};

class AssetsWindow : public UniqueWindow {


public:
	AssetsWindow(std::string_view name);

	virtual void ImguiDraw();
	virtual ~AssetsWindow() = default;

private:
	AssetFileEntry m_root;
	std::string m_currentPath{};

	void ReloadEntries();
	void DrawAsset(PodEntry* podEntry);
	void AppendEntry(AssetFileEntry* entry, const std::string& pathToHere);
	AssetFileEntry* GetPathEntry(const std::string& path);
};

} // namespace ed
