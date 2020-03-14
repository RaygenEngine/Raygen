#include "pch.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "editor/windows/general/EdAssetsWindow.h"
#include "asset/AssetManager.h"
#include "core/StringUtl.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiUtil.h"
#include "reflection/ReflectionTools.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/utl/EdAssetUtils.h"
#include "editor/misc/NativeFileBrowser.h"
#include "reflection/ReflEnum.h"

#include <spdlog/fmt/fmt.h>
#include <imgui_internal.h>

namespace ed {
using namespace assetentry;
AssetsWindow::AssetsWindow(std::string_view name)
	: UniqueWindow(name)
{
	ChangeDir(&m_root);
}

void AssetsWindow::ReloadEntries()
{
	auto& pods = AssetHandlerManager::Z_GetPods();

	m_root.folders.clear();
	m_root.files.clear();

	for (auto& pod : pods) {
		[[unlikely]] if (pod->transient) { continue; }

		auto uri = uri::GetDir(pod->path);
		if (uri.size() == 0) {
			m_root.AddFile(pod.get());
			continue;
		}

		auto parts = str::split(uri, "/");
		auto* currentNode = &m_root;
		for (auto& subpath : parts) {
			currentNode = currentNode->FindOrAddFolder(subpath);
		}
		currentNode->AddFile(pod.get());
	}
	ChangeDir(&m_root);
}


void AssetsWindow::DrawDirectoryToList(FolderEntry* folder)
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

	bool open = ImGui::TreeNodeEx(folder->name.c_str(), nodeFlags);
	ImGui::PopStyleVar(2);
	if (ImGui::IsItemClicked()) {
		ChangeDir(folder);
	}
	if (open) {
		for (auto& c : folder->folders) {
			DrawDirectoryToList(c.second.get());
		}
		ImGui::TreePop();
	}
}

void AssetsWindow::ChangeDir(FolderEntry* newDirectory)
{
	m_currentFolder = newDirectory;
	// LIFO the pointers to avoid extreme text copies
	std::vector<FolderEntry*> pathFolders;
	while (!newDirectory->IsRoot()) {
		pathFolders.push_back(newDirectory);
		newDirectory = newDirectory->parent;
	}

	m_currentPath = "";
	for (auto e : pathFolders) {
		m_currentPath += e->name + "/";
	}
}

void AssetsWindow::CreateFolder(const std::string& name, assetentry::FolderEntry* where = nullptr)
{
	if (where == nullptr) {
		where = m_currentFolder;
	}
	where->FindOrAddFolder(name);
}

static bool selected = false;

struct NoFunc {
	void operator()() {}
};

template<bool IsFolder, typename OnOpen = NoFunc, typename OnEndGroup = NoFunc, typename OnDrag = NoFunc>
void Draw(const char* iconTxt, const char* nameTxt, OnOpen onOpen = {}, OnEndGroup onEndGroup = {}, OnDrag onDrag = {})
{
	constexpr float padding = 6.f;
	constexpr float c_itemWidth = 110.f;
	constexpr float c_itemHeight = 100.f;
	constexpr float c_maxLabelWidth = 100.f;


	const float scaledLabelWidth = c_maxLabelWidth * ImGui::GetCurrentWindow()->FontWindowScale;
	const ImVec2 scaledSize = ImVec2(c_itemWidth, c_itemHeight) * ImGui::GetCurrentWindow()->FontWindowScale;

	ImVec2 cursBegin = ImGui::GetCursorPos();

	ImGui::Selectable("##AssetEntrySelectableGroup", &selected, 0, scaledSize);
	onEndGroup();
	onDrag();
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) { // Open
			onOpen();
		}
		else { // Select
			   // TODO: ASSETS:
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

			ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
				ImVec2(pos.x + centerOffset, pos.y), ImGui::GetColorU32(ImGuiCol_Text), nameTxt, nullptr,
				scaledLabelWidth, &clip_rect);
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
	ImGui::PushID(folderEntry);
	Draw<true>(U8(FA_FOLDER), folderEntry->name.c_str(), [&]() { ChangeDir(folderEntry); });
	ImGui::PopID();
}

void AssetsWindow::DrawAsset(PodEntry* assetEntry)
{

	if (m_fileFilter.IsActive() && !m_fileFilter.PassFilter(assetEntry->path.c_str())
		&& !m_fileFilter.PassFilter(assetEntry->name.c_str())
		&& !m_fileFilter.PassFilter(assetEntry->metadata.originalImportLocation.c_str())
		&& !m_fileFilter.PassFilter(assetEntry->type.name_str().c_str())) {
		return;
	}

	ImGui::PushID(assetEntry);
	Draw<false>(
		U8(assetEntry->GetClass()->GetIcon()), assetEntry->name.c_str(), []() {},
		[&]() { ed::asset::MaybeHoverTooltip(assetEntry); }, [&]() { ImEd::CreateTypedPodDrag(assetEntry); });
	ImGui::PopID();
}

void AssetsWindow::ImguiDraw()
{
	static bool init = false;
	if (ImEd::Button(ETXT(FA_REFRESH_A, "Reload")) || !init) {
		ReloadEntries();
		init = true;
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
		if (auto files = ed::NativeFileBrowser::OpenFileMultiple({ "gltf" })) {
			for (auto& path : *files) {
				// TODO: IMPORTERS
				if (path.extension() == ".gltf") {
					AssetImporterManager::OLD_ResolveOrImport<ModelPod>(path, "", fs::path(m_currentPath));
				}
				else {
					AssetImporterManager::OLD_ResolveOrImport<ImagePod>(path, "", fs::path(m_currentPath));
				}
			}
			ReloadEntries();
		}
	}

	ImGui::SameLine();
	if (m_fileFilter.IsActive()) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
	}
	if (ImEd::Button(ETXT(FA_SEARCH, "Filter"))) {
		ImGui::OpenPopup("##AssetFilterPopup");
	}
	if (m_fileFilter.IsActive()) {
		ImGui::PopStyleColor();
	}


	if (ImGui::BeginPopup("##AssetFilterPopup", ImGuiMouseButton_Left)) {
		if (ImGui::Button(U8(FA_TIMES))) {
			m_fileFilter.Clear();
		}
		ImGui::SameLine();
		m_fileFilter.Draw(U8(FA_SEARCH));

		ImGui::EndPopup();
	}


	ImEd::HSpace();
	ImGui::SameLine();
	ImGui::Text("./%s", m_currentPath.c_str());
	ImGui::Separator();

	ImGui::Columns(2, NULL, true);

	if (ImGui::BeginChild("AssetsFolderView", ImVec2(0, -5.f))) {
		DrawDirectoryToList(&m_root);
	}
	ImGui::EndChild();


	ImGui::NextColumn();
	ImGui::Indent(8.f);
	if (ImGui::BeginChild("AssetsFileView", ImVec2(0, -5.f))) {
		for (auto& e : m_currentFolder->folders) {
			DrawFolder(e.second.get());
		}

		for (auto& e : m_currentFolder->files) {
			DrawAsset(e.second->entry);
		}
	}
	ImGui::Unindent(8.f);
	ImGui::EndChild();
	ImGui::Columns(1);
}

void AssetsWindow::ImportFiles(std::vector<fs::path>&& files)
{
	for (auto& file : files) {
		AssetFrontEndManager::Import(file);
	}
	ReloadEntries();
}

} // namespace ed
