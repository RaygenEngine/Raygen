#include "pch/pch.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "editor/windows/general/EdAssetsWindow.h"
#include "asset/AssetManager.h"
#include "core/StringAux.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiUtil.h"
#include "reflection/ReflectionTools.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/utl/EdAssetUtils.h"
#include "editor/misc/NativeFileBrowser.h"

#include <magic_enum.hpp>
#include <spdlog/fmt/fmt.h>
#include <imgui_internal.h>

namespace ed {
AssetsWindow::AssetsWindow(std::string_view name)
	: UniqueWindow(name)
{
	ReloadEntries();
}

void AssetsWindow::ReloadEntries()
{
	auto& pods = AssetHandlerManager::Z_GetPods();

	for (auto& pod : pods) {
		[[unlikely]] if (pod->transient) { continue; }

		auto parts = str::Split(uri::GetDir(pod->path), "/");
		auto* currentNode = &m_root;
		if (parts.size() > 1) {
			for (auto& subpath : parts) {
				currentNode->containsDirectories = true;
				// TODO: ASSETS What a mess, simplify
				currentNode
					= currentNode->children.emplace(std::string(subpath), std::make_unique<AssetFileEntry>(subpath))
						  .first->second.get();
			}
		}
		auto filename = uri::GetFilename(pod->path);
		currentNode->children.emplace(filename, std::make_unique<AssetFileEntry>(filename, pod.get()));
	}
}

AssetFileEntry* AssetsWindow::GetPathEntry(const std::string& path)
{
	if (path.size() <= 1) {
		return &m_root;
	}

	auto parts = str::Split(path, "/");

	auto* currentNode = &m_root;
	if (parts.size() == 1) {
		return currentNode;
	}

	for (auto& subpath : parts) {
		// TODO: What a mess, simplify
		auto it = currentNode->children.find(std::string(subpath));
		if (it == currentNode->children.end()) {
			return nullptr;
		}
		currentNode = it->second.get();
	}

	return currentNode;
}

void AssetsWindow::AppendEntry(AssetFileEntry* entry, const std::string& pathToHere)
{
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
								   | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

	std::string thisPath = pathToHere + "/" + entry->subpath;


	if (!entry->entry) {
		if (m_currentPath == thisPath) {
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		}
		if (!entry->containsDirectories) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 3.f });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 15.f, 0.f });
		bool open = ImGui::TreeNodeEx(entry->subpath.c_str(), nodeFlags);
		ImGui::PopStyleVar(2);
		if (ImGui::IsItemClicked()) {
			m_currentPath = thisPath;
		}
		if (open) {
			for (auto& c : entry->children) {
				AppendEntry(c.second.get(), pathToHere + "/" + entry->subpath);
			}
			ImGui::TreePop();
		}
	}
}

void AssetsWindow::DrawAsset(PodEntry* assetEntry)
{
	ImGui::PushID(static_cast<int32>(assetEntry->uid));

	constexpr float padding = 6.f;
	constexpr float c_itemWidth = 110.f;
	constexpr float c_itemHeight = 100.f;
	constexpr float c_maxLabelWidth = 100.f;


	const float scaledLabelWidth = c_maxLabelWidth * ImGui::GetCurrentWindow()->FontWindowScale;
	const ImVec2 scaledSize = ImVec2(c_itemWidth, c_itemHeight) * ImGui::GetCurrentWindow()->FontWindowScale;

	ImVec2 cursBegin = ImGui::GetCursorPos();

	static bool selected = false; // TODO: Hack
	ImGui::Selectable("##AssetEntrySelectableGroup", &selected, 0, scaledSize);
	ImGui::SetCursorPos(cursBegin);
	ImGui::BeginGroup();

	auto iconTxt = U8(assetEntry->GetClass()->GetIcon());
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
		auto nameTxt = assetEntry->name.c_str();
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
	ed::asset::MaybeHoverTooltip(assetEntry);


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


	ImGui::PopID();
}

void AssetsWindow::ImguiDraw()
{
	static bool init = false;
	if (ImEd::Button("Reload") || !init) {
		ReloadEntries();
		init = true;
	}
	size_t prevDirPos = m_currentPath.rfind('/');
	const bool CanBack = prevDirPos > 1;
	ImGui::SameLine();
	if (ImEd::Button("Back") && CanBack) {
		m_currentPath.erase(prevDirPos);
	}
	ImGui::SameLine();
	if (ImEd::Button(U8(FA_FILE_IMPORT u8"  Import Files"))) {
		if (auto files = ed::NativeFileBrowser::OpenFileMultiple({ "gltf,png,jpg,tiff" })) {
			for (auto& path : *files) {
				// WIP: ASSETS
				if (path.extension() == ".gltf") {
					AssetImporterManager::ResolveOrImport<ModelPod>(path, "", fs::path(m_currentPath));
				}
				else {
					AssetImporterManager::ResolveOrImport<ImagePod>(path, "", fs::path(m_currentPath));
				}
			}
			ReloadEntries();
		}
	}
	ImEd::HSpace();
	ImGui::SameLine();
	ImGui::Text(m_currentPath.c_str());
	ImGui::Separator();

	ImGui::Columns(2, NULL, true);

	if (ImGui::BeginChild("AssetsFolderView", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * 1))) {
		for (auto& child : m_root.children) {
			AppendEntry(child.second.get(), "");
		}
	}
	ImGui::EndChild();


	ImGui::NextColumn();
	ImGui::Indent(8.f);
	if (ImGui::BeginChild("AssetsFileView", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * 1))) {
		AssetFileEntry* entry = GetPathEntry(m_currentPath);
		for (auto& e : entry->children) {
			if (e.second->entry == nullptr) {
				continue;
			}
			DrawAsset(e.second->entry);
		}
	}
	ImGui::Unindent(8.f);
	ImGui::EndChild();
	ImGui::Columns(1);
}

void AssetsWindow::ImportFiles(std::vector<std::string>&& files)
{

	for (auto& file : files) {
		auto path = fs::path(file);
		if (path.extension() == ".gltf") {
			AssetImporterManager::ResolveOrImport<ModelPod>(path, "", fs::path(m_currentPath));
		}
	}
	ReloadEntries();
}

} // namespace ed
