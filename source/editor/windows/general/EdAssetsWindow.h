#pragma once
#include "editor/windows/EdWindow.h"
#include "engine/profiler/ProfilerSetup.h"
#include "asset/AssetManager.h"
#include "imgui/imgui.h"
#include <map>

namespace ed {
namespace assetentry {
	struct FileEntry;
	struct FolderEntry;


	using FileContainer = std::map<std::string, UniquePtr<FileEntry>, str::LessInsensitive>;
	using FolderContainer = std::map<std::string, UniquePtr<FolderEntry>, str::LessInsensitive>;

	struct FileEntry {
		std::string name;
		PodEntry* entry{ nullptr };
		FolderEntry* parent;

		FileEntry() {}
		FileEntry(PodEntry* e, FolderEntry* parent)
			: entry(e)
			, name(e->GetName())
			, parent(parent)
		{
			CLOG_ABORT(e == nullptr, "Invalid entry");
		}
	};

	struct FolderEntry {
		std::string name;

		FileContainer files;
		FolderContainer folders;
		FolderEntry* parent;

		FolderEntry(const std::string& name, FolderEntry* parent)
			: name(name)
			, parent(parent)
		{
		}

		bool IsRoot() const { return parent == nullptr; }

		FolderEntry* FindOrAddFolder(std::string_view folderName)
		{
			auto it = folders.find(folderName);

			if (it != folders.end()) {
				return it->second.get();
			}

			return folders.emplace_hint(it, folderName, std::make_unique<FolderEntry>(std::string(folderName), this))
				->second.get();
		}

		FileEntry* AddFile(PodEntry* entry)
		{
			const auto name = entry->GetName();
			auto it = files.find(name);

			if (it != files.end()) {
				return it->second.get();
			}

			return files.emplace_hint(it, name, std::make_unique<FileEntry>(entry, this))->second.get();
		}
	};
} // namespace assetentry

class AssetsWindow : public UniqueWindow {


public:
	AssetsWindow(std::string_view name);

	virtual void ImguiDraw();
	virtual ~AssetsWindow() = default;


	void ImportFiles(std::vector<fs::path>&& files);

private:
	assetentry::FolderEntry m_root{ "Assets", nullptr };
	assetentry::FolderEntry* m_currentFolder{};
	std::string m_currentPath{};
	ImGuiTextFilter m_fileFilter{};


	void ReloadEntries();

	void DrawFolder(assetentry::FolderEntry* folderEntry);
	void DrawAsset(PodEntry* podEntry);

	void DrawDirectoryToList(assetentry::FolderEntry* entry);
	void ChangeDir(assetentry::FolderEntry* newDirectory);

	void CreateFolder(const std::string& name, assetentry::FolderEntry* where);
};

} // namespace ed
