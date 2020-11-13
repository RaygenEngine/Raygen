#pragma once
#include "assets/PodEntry.h"
#include "core/StringUtl.h"
#include "editor/windows/EdWindow.h"
#include "engine/Logger.h"
#include "engine/profiler/ProfilerSetup.h"

#include <imgui/imgui.h>
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

		[[nodiscard]] bool IsRoot() const { return parent == nullptr; }

		FolderEntry* FindOrAddFolder(std::string_view folderName)
		{
			auto it = folders.find(folderName);

			if (it != folders.end()) {
				return it->second.get();
			}

			return folders.emplace_hint(it, folderName, std::make_unique<FolderEntry>(std::string(folderName), this))
				->second.get();
		}

		FolderEntry* FindFolder(std::string_view folderName)
		{
			auto it = folders.find(folderName);

			if (it != folders.end()) {
				return it->second.get();
			}

			return nullptr;
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

		// Recurse all files *& files in subfolders under this directory
		template<typename Lambda>
		void ForEachFile(Lambda&& function)
		{
			for (auto& file : files) {
				function(file.second);
			}
			for (auto& folder : folders) {
				folder.second->ForEachFile(function);
			}
		}

		// Recurse all folders & subfolders under this directory
		template<typename Lambda>
		void ForEachFolder(Lambda&& function)
		{
			for (auto& folder : folders) {
				function(folder.second.get());
				folder.second->ForEachFolder(function);
			}
		}

		[[nodiscard]] std::string CalculatePathFromRoot(bool includeGenData)
		{
			FolderEntry* current = this;
			std::vector<FolderEntry*> pathFolders;
			while (!current->IsRoot()) {
				pathFolders.push_back(current);
				current = current->parent;
			}

			// NOTE: assumes gen-data
			std::string path = includeGenData ? "gen-data/" : "";

			for (auto it = pathFolders.rbegin(); it != pathFolders.rend(); ++it) {
				path += (*it)->name + "/";
			}
			return path;
		}
	};
} // namespace assetentry

class AssetsWindow : public UniqueWindow {


public:
	AssetsWindow(std::string_view name);

	virtual void ImguiDraw();
	virtual ~AssetsWindow() = default;


	void ImportFiles(std::vector<fs::path>&& files);

	static void RefreshEntries();

private:
	assetentry::FolderEntry m_root{ "gen-data", nullptr };
	assetentry::FolderEntry* m_currentFolder{};
	std::string m_currentPath{};
	ImGuiTextFilter m_fileFilter{};


	bool m_wasRenamingPrevFrame{ false };
	std::string m_renameString;
	std::string m_newAssetNameString;
	TypeId m_newAssetTypeId;

	PodEntry* m_selectedEntry{ nullptr };


	BoolFlag m_resizeColumn{ true };
	BoolFlag m_reloadEntries{ true };
	bool m_reopenPath{ true };

	void ReloadEntries();
	void ReloadEntriesImpl();

	void DrawFolder(assetentry::FolderEntry* folderEntry);
	void DrawAsset(PodEntry* podEntry);

	void DrawDirectoryToList(assetentry::FolderEntry* entry, bool isInPath);
	void ChangeDir(assetentry::FolderEntry* newDirectory);
	void ChangeDirPath(std::string_view newDirectory);

	void RunEmptySpaceContext();

	void CreateFolder(const std::string& name, assetentry::FolderEntry* where);

	void RunFileEntryContext(PodEntry* entry);
	void RunFolderEntryContext(assetentry::FolderEntry* folder);

	void RunPostFolder(assetentry::FolderEntry* folder);
	void RunKeyboard();
};

} // namespace ed
