#include "pch.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "EdAssetsWindow.h"

#include "assets/Assets.h"
#include "core/StringUtl.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/imgui/ImguiUtil.h"
#include "editor/misc/NativeFileBrowser.h"
#include "editor/utl/EdAssetUtils.h"
#include "reflection/ReflectionTools.h"
#include "reflection/ReflEnum.h"
#include "reflection/PodTools.h"
#include "editor/EditorObject.h"
#include "engine/Input.h"

#include <imgui/imgui_internal.h>
#include <spdlog/fmt/fmt.h>

namespace ed {
using namespace assetentry;
AssetsWindow::AssetsWindow(std::string_view name)
	: UniqueWindow(name)
{
	ChangeDir(&m_root);
}

namespace {
	FolderEntry* FindOrCreatePath(FolderEntry* root, std::string_view path, bool skipCreation = false)
	{
		// NOTE: May contain bugs because it assumes gen-data assets only.
		auto parts = str::split(path, "/");
		parts.erase(parts.begin());

		auto* currentNode = root;
		for (auto& subpath : parts) {
			if (!skipCreation) {
				currentNode = currentNode->FindOrAddFolder(subpath);
			}
			else {
				auto nextNode = currentNode->FindFolder(subpath);
				if (nextNode == nullptr) {
					return currentNode;
				}
				currentNode = nextNode;
			}
		}
		return currentNode;
	}


} // namespace

void AssetsWindow::ReloadEntries()
{
	m_reloadEntries.Set();
}

void AssetsWindow::ReloadEntriesImpl()
{
	auto& pods = AssetHandlerManager::Z_GetPods();

	auto prevDirectory = m_currentPath; // Store by copy, currentpath gets changed on reloadentries

	m_root.folders.clear();
	m_root.files.clear();

	for (auto& pod : pods) {
		[[unlikely]] if (pod->transient) { continue; }

		auto uri = uri::GetDir(pod->path);
		if (uri.size() == 0) {
			m_root.AddFile(pod.get());
			continue;
		}

		FindOrCreatePath(&m_root, uri)->AddFile(pod.get());
	}
	ChangeDirPath(prevDirectory);
}

namespace {
	bool IsInPath(FolderEntry* entry, FolderEntry* currentFolder)
	{
		FolderEntry* current = currentFolder;

		if (entry->IsRoot()) {
			return true;
		}

		while (!current->IsRoot()) {
			if (current == entry) {
				return true;
			}
			current = current->parent;
		}
		return false;
	}
} // namespace

void AssetsWindow::DrawDirectoryToList(FolderEntry* folder, bool isInPath)
{
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
								   | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;


	if (m_currentFolder == folder) {
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}
	if (folder->folders.size() == 0) {
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 3.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 15.f, 0.f });

	if (isInPath && m_reopenPath) {
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	}

	bool open = ImGui::TreeNodeEx(folder->name.c_str(), nodeFlags);
	ImGui::PopStyleVar(2);
	if (ImGui::IsItemClicked()) {
		ChangeDir(folder);
	}
	RunPostFolder(folder);
	if (open) {
		for (auto& c : folder->folders) {
			bool nextIsInPath = false;
			if (isInPath) {
				nextIsInPath = IsInPath(c.second.get(), m_currentFolder);
			}
			DrawDirectoryToList(c.second.get(), nextIsInPath);
		}
		ImGui::TreePop();
	}
}

void AssetsWindow::ChangeDir(FolderEntry* newDirectory)
{
	m_currentFolder = newDirectory;
	m_currentPath = newDirectory->CalculatePathFromRoot(true);
	m_reopenPath = true;
}

void AssetsWindow::ChangeDirPath(std::string_view newDirectory)
{
	ChangeDir(FindOrCreatePath(&m_root, newDirectory, true));
}

void AssetsWindow::CreateFolder(const std::string& name, assetentry::FolderEntry* where = nullptr)
{
	if (where == nullptr) {
		where = m_currentFolder;
	}
	where->FindOrAddFolder(name);
}


struct NoFunc {
	void operator()() {}
};

template<bool IsFolder, typename OnOpen = NoFunc, typename OnEndGroup = NoFunc, typename OnDrag = NoFunc,
	typename OnSelect = NoFunc>
void Draw(const char* iconTxt, const char* nameTxt, OnOpen onOpen = {}, OnEndGroup onEndGroup = {}, OnDrag onDrag = {},
	bool selectedVisual = false, OnSelect onSelect = {}, bool needsSave = false)
{
	constexpr float padding = 6.f;
	constexpr float c_itemWidth = 90.f;
	constexpr float c_itemHeight = 86.f;
	constexpr float c_maxLabelWidth = 90.f;

	std::string nameBuffer;
	if (needsSave) {
		nameBuffer = "*" + std::string(nameTxt);
		nameTxt = nameBuffer.c_str();
	}


	const float scaledLabelWidth = c_maxLabelWidth * ImGui::GetCurrentWindow()->FontWindowScale;
	const ImVec2 scaledSize = ImVec2(c_itemWidth, c_itemHeight) * ImGui::GetCurrentWindow()->FontWindowScale;

	ImVec2 cursBegin = ImGui::GetCursorPos();

	ImGui::Selectable("##AssetEntrySelectableGroup", &selectedVisual, 0, scaledSize);


	onEndGroup();
	onDrag();
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) { // Open
			onOpen();
		}
		else { // Select
			onSelect();
		}
	}

	ImGui::SetCursorPos(cursBegin);
	ImGui::BeginGroup();


	// Icon
	{
		ImGui::BeginGroup();
		ImGui::PushFont(ImguiImpl::s_AssetIconFont);
		float width = ImGui::CalcTextSize(iconTxt).x;
		float cursPos = ((scaledSize.x - width) / 2.f) + cursBegin.x;
		ImGui::SetCursorPosX(cursPos);
		ImGui::TextUnformatted(iconTxt);
		ImGui::PopFont();
		ImGui::EndGroup();
	}


	// Label
	{
		const float txtWidth = std::min(ImGui::CalcTextSize(nameTxt).x, scaledLabelWidth);

		const float centerOffset = ((scaledSize.x - txtWidth) / 2.f);

		const ImVec2 labelSize = ImVec2(scaledSize.x, ImGui::GetFontSize() * 2.f);
		ImVec2 pos = ImGui::GetCursorScreenPos();

		// Hitbox
		{
			ImRect bb(pos, pos + labelSize);
			ImGui::ItemSize(labelSize, 0.0f);
			ImGui::ItemAdd(bb, 0);
		}
		// Drawing
		{
			ImVec4 clip_rect(pos.x, pos.y, pos.x + labelSize.x, pos.y + labelSize.y);

			ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.91f,
				ImVec2(pos.x + centerOffset, pos.y),
				needsSave ? ImGui::ColorConvertFloat4ToU32(EdColor::Red) : ImGui::GetColorU32(ImGuiCol_Text), nameTxt,
				nullptr, scaledLabelWidth, &clip_rect);
		}
	}
	ImGui::EndGroup();


	const float nextItemStartX = cursBegin.x + scaledSize.x + padding;
	const float nextItemEndX = nextItemStartX + scaledSize.x;
	bool needsWrapping = (ImGui::GetWindowSize().x - nextItemEndX) < 0.f;

	if (!needsWrapping) {
		ImGui::SameLine();
		ImGui::SetCursorPosX(nextItemStartX);
	}
	else {
		ImGui::SetCursorPosY(cursBegin.y + scaledSize.y + padding);
	}
}

void AssetsWindow::DrawFolder(assetentry::FolderEntry* folderEntry)
{
	if (m_fileFilter.IsActive() && !m_fileFilter.PassFilter(folderEntry->name.c_str())) {
		return;
	}

	ImGui::PushID(folderEntry);

	Draw<true>(
		U8(FA_FOLDER), folderEntry->name.c_str(), [&]() { ChangeDir(folderEntry); },
		[&]() { RunPostFolder(folderEntry); });
	ImGui::PopID();
}

void AssetsWindow::DrawAsset(PodEntry* assetEntry)
{
	if (m_fileFilter.IsActive() //
		&& !m_fileFilter.PassFilter(assetEntry->name.c_str())
		&& !m_fileFilter.PassFilter(assetEntry->type.name_str().c_str())
		&& !m_fileFilter.PassFilter(
			fs::path(assetEntry->metadata.originalImportLocation).filename().string().c_str())) {
		return;
	}

	ImGui::PushID(assetEntry);
	Draw<false>(
		U8(assetEntry->GetClass()->GetIcon()), assetEntry->name.c_str(),
		// On Open
		[&]() { EditorObject->m_windowsComponent.OpenAsset(assetEntry); },
		// Post Element
		[&]() {
			RunFileEntryContext(assetEntry);
			//
			ed::asset::MaybeHoverTooltip(assetEntry);
		},
		[&]() { ImEd::CreateTypedPodDrag(assetEntry); },                        //
																				// Selection Parameters
		m_selectedEntry == assetEntry, [&]() { m_selectedEntry = assetEntry; }, //
		assetEntry->requiresSave);
	ImGui::PopID();
}

void AssetsWindow::ImguiDraw()
{
	if (ImEd::Button(ETXT(FA_REFRESH_A, "Reload"))) {
		ReloadEntries();
	}

	if (m_reloadEntries.Access()) {
		ReloadEntriesImpl();
	}


	ImGui::SameLine();
	if (ImEd::Button(ETXT(FA_SAVE, "Save All"))) {
		AssetHandlerManager::SaveAll();
	}

	ImGui::SameLine();
	if (ImEd::Button(ETXT(FA_ARROW_UP, "Back")) && !m_currentFolder->IsRoot()) {
		ChangeDir(m_currentFolder->parent);
	}

	bool isFirstOpen = false;
	ImGui::SameLine();
	if (ImEd::Button(ETXT(FA_FOLDER_PLUS, "Create Folder"))) {
		ImGui::OpenPopup("##CreateFolderInputTextPopup");
		isFirstOpen = true;
	}

	if (ImGui::BeginPopup("##CreateFolderInputTextPopup", ImGuiMouseButton_Left)) {
		static std::string str;
		if (isFirstOpen) {
			str = "New Folder";
			ImGui::SetKeyboardFocusHere();
		}

		if (ImGui::InputText("##CreateFolderInputText", &str, ImGuiInputTextFlags_EnterReturnsTrue,
				ImEd::AssetNameFilter::FilterImGuiLetters)) {
			CreateFolder(str);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::SameLine();
	if (ImEd::Button(ETXT(FA_FILE_IMPORT, "Import Files"))) {
		if (auto files = ed::NativeFileBrowser::OpenFileMultiple({ "*;gltf;png,jpg,jpeg,tiff;vert,frag" })) {

			auto importDirectory = m_currentPath; // Store by copy, currentpath gets changed on reloadentries
			AssetImporterManager->SetPushPath(m_currentPath);
			for (auto& path : *files) {
				Assets::Import(path);
			}
			AssetImporterManager->PopPath();
			ReloadEntries();
			ChangeDirPath(importDirectory);
		}
	}

	ImGui::SameLine();
	bool wasActive = m_fileFilter.IsActive();
	if (wasActive) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
	m_fileFilter.Draw(U8(FA_SEARCH), 140.f);
	ImGui::PopStyleVar();

	// PERF: Cache search results. Huge hit when having a lot of assets.
	if (m_fileFilter.IsActive()) {
		ImGui::SameLine();
		if (ImEd::Button(U8(FA_TIMES))) {
			m_fileFilter.Clear();
		}
	}
	if (wasActive) {
		ImGui::PopStyleColor();
	}


	ImEd::HSpace();
	ImGui::SameLine();
	ImGui::Text("./%s", m_currentPath.c_str());
	ImGui::Separator();

	ImGui::Columns(2, NULL, true);

	if (m_resizeColumn.Access()) {
		ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.25f);
	}


	if (ImGui::BeginChild("AssetsFolderView", ImVec2(0, -5.f))) {
		DrawDirectoryToList(&m_root, true);
		m_reopenPath = false;
	}
	ImGui::EndChild();


	ImGui::NextColumn();
	ImGui::Indent(8.f);
	if (ImGui::BeginChild("AssetsFileView", ImVec2(0, -5.f))) {
		if (m_fileFilter.IsActive() && strlen(m_fileFilter.InputBuf) > 2) {
			m_currentFolder->ForEachFolder([&](FolderEntry* folder) { //
				DrawFolder(folder);
			});

			m_currentFolder->ForEachFile([&](auto& file) { //
				DrawAsset(file->entry);
			});
		}
		else {
			for (auto& e : m_currentFolder->folders) {
				DrawFolder(e.second.get());
			}

			for (auto& e : m_currentFolder->files) {
				DrawAsset(e.second->entry);
			}
		}
	}
	ImGui::Unindent(8.f);
	RunKeyboard();
	RunEmptySpaceContext();
	ImGui::EndChild();
	ImGui::Columns(1);
}

void AssetsWindow::RunEmptySpaceContext()
{
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered()) {
		m_selectedEntry = nullptr;
	}

	if (ImGui::BeginPopupContextWindow("AssetsGenericContextRightClickWindow", ImGuiMouseButton_Right, false)) {
		mti::TypeId newPodType;

		if (ImGui::IsWindowAppearing()) {
			m_selectedEntry = nullptr;
		}

		podtools::ForEachPodType([&]<typename PodType>() {
			//
			auto itemName = PodType::StaticClass().GetNameStr();

			float sizeX = ImMax(ImGui::CalcTextSize(itemName.c_str(), NULL, true).x, 150.f); // Pad width for better ux

			if (ImGui::Selectable(itemName.c_str(), false,
					ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_DrawFillAvailWidth
						| ImGuiSelectableFlags_PressedOnRelease,
					ImVec2(sizeX, 0.0f))) {
				ImGui::OpenPopup("NewAssetNamePopup");
				m_newAssetNameString = itemName;
				m_newAssetTypeId = mti::GetTypeId<PodType>();
			}
		});


		if (ImGui::BeginPopup("NewAssetNamePopup")) {
			if (ImGui::IsWindowAppearing()) {
				ImGui::SetKeyboardFocusHere();
			}

			bool make = false;

			ImGui::SetNextItemWidth(150.f);
			make |= ImGui::InputText("###NewAssetNameTxt", &m_newAssetNameString, ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::SameLine();
			make |= ImGui::Button("OK");

			if (make) {
				podtools::VisitPodHash(m_newAssetTypeId.hash(), [&]<typename PodType>() {
					auto& [entry, _] = AssetHandlerManager::CreateEntry<PodType>(m_currentPath + m_newAssetNameString);
					m_selectedEntry = entry;
				});
				ReloadEntries();
				ImGui::ClosePopupToLevel(0, true);
			}


			ImGui::EndPopup();
		}

		ImGui::EndPopup();
	}
}

void AssetsWindow::ImportFiles(std::vector<fs::path>&& files)
{
	for (auto& file : files) {
		Assets::Import(file);
	}
	ReloadEntries();
}

void AssetsWindow::RunFileEntryContext(PodEntry* entry)
{
	bool isContextOpen = ImGui::BeginPopupContextItem("AssetsContextRightClickWindow");
	if (isContextOpen) {
		m_selectedEntry = entry;
		if (!m_wasRenamingPrevFrame) {
			m_wasRenamingPrevFrame = true;
			m_renameString = entry->name;
		}

		if (entry->IsA<MaterialArchetype>()) {
			if (ImGui::MenuItem("Make Instance")) {
				MaterialArchetype::MakeInstancePod(entry->GetHandleAs<MaterialArchetype>());
				ReloadEntries();
			}
			ImGui::Separator();
		}

		if (ImGui::InputText("Rename", &m_renameString, ImGuiInputTextFlags_EnterReturnsTrue)) {
			AssetHandlerManager::RenameEntry(entry, std::string(uri::GetDir(entry->path)) + m_renameString);
			AssetHandlerManager::SaveAll();
			ImGui::CloseCurrentPopup();
			ReloadEntries();
		}

		if (ImGui::Button("Duplicate")) {
			auto newEntry = AssetHandlerManager::Duplicate(entry);
			AssetHandlerManager::SaveToDisk(newEntry);
			ImGui::CloseCurrentPopup();
			ReloadEntries();
		}

		if (ImGui::Button("Edit")) {
			EditorObject->m_windowsComponent.OpenAsset(entry);
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Save")) {
			AssetHandlerManager::SaveToDisk(entry);
		}


		if (ImGui::MenuItem("Export")) {
			if (auto file = NativeFileBrowser::SaveFile(); file) {
				AssetHandlerManager::ExportToLocation(entry, *file);
			}
		}

		ImGui::EndPopup();
	}
	else {
		m_wasRenamingPrevFrame = false;
	}
}

namespace {
	void RenameFolder(FolderEntry* entry, const std::string& newName)
	{
		// PERF: can certainly reduce string operations here, should only be relevant for renaming thousands of
		// assets though
		std::string initialString = entry->CalculatePathFromRoot(true);
		std::string newString = "gen-data/" + newName;

		if (!newName.ends_with('/')) {
			newString += '/';
		}

		entry->ForEachFile([&](UniquePtr<FileEntry>& file) { //
			const char* begin = file->entry->path.data() + initialString.size();
			std::string newPath = fmt::format("{}{}", newString, begin);
			AssetHandlerManager::RenameEntry(file->entry, newPath);

		});
		AssetHandlerManager::SaveAll();
	}
} // namespace

void AssetsWindow::RunFolderEntryContext(assetentry::FolderEntry* folder)
{
	bool isRenaming = ImGui::BeginPopupContextItem("AssetsContextRightClickWindow");
	if (isRenaming) {
		if (!m_wasRenamingPrevFrame) {
			m_wasRenamingPrevFrame = true;
			m_renameString = folder->name;
		}

		if (ImGui::InputText("Rename", &m_renameString, ImGuiInputTextFlags_EnterReturnsTrue)) {
			RenameFolder(folder, std::string(uri::GetDir(folder->CalculatePathFromRoot(true))) + m_renameString);
			ReloadEntries();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	else {
		m_wasRenamingPrevFrame = false;
	}
}

void AssetsWindow::RunPostFolder(FolderEntry* folder)
{
	RunFolderEntryContext(folder);
	if (auto dropentry = ImEd::AcceptGenericPodDrop()) {
		AssetHandlerManager::RenameEntry(dropentry, folder->CalculatePathFromRoot(true) + dropentry->name);
		AssetHandlerManager::SaveAll();
		ReloadEntries();
	}
	if (auto dropFolder = ImEd::AcceptDropObject<FolderEntry>("ASSET_WINDOW_FOLDER")) {
		LOG_REPORT("Dropped: {} on {}", dropFolder->name, folder->name);
		RenameFolder(dropFolder, folder->CalculatePathFromRoot(false) + dropFolder->name + "/");
		ReloadEntries();
	}
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
		ImGui::PushFont(ImguiImpl::s_AssetIconFont);
		ImGui::TextUnformatted(U8(FA_FOLDER));
		ImGui::PopFont();
		ImGui::TextUnformatted(std::string(folder->name).c_str());

		ImEd::EndDragDropSourceObject(folder, "ASSET_WINDOW_FOLDER");
	}
}

void ed::AssetsWindow::RunKeyboard()
{
	if (!ImGui::IsWindowFocused()) {
		return;
	}

	PodEntry* entry = m_selectedEntry;
	if (!entry) {
		return;
	}

	if (ImGui::IsKeyPressedMap(ImGuiKey_Enter) || ImGui::IsKeyPressedMap(ImGuiKey_KeyPadEnter)) {
		// Open
		ed::asset::OpenForEdit(entry);
	}

	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(83)) {
		AssetHandlerManager::SaveToDisk(entry);
	}

	if (Input.IsJustPressed(Key::F2)) {
		// Inline Rename
	}
}
} // namespace ed
