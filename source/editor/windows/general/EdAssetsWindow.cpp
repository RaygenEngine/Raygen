#include "pch/pch.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "asset/AssetManager.h"
#include "core/StringAux.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiUtil.h"
#include "reflection/ReflectionTools.h"
#include "editor/imgui/ImguiImpl.h"

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
		auto parts = str::Split(uri::GetDir(pod->path), "/");
		auto* currentNode = &m_root;
		if (parts.size() > 1) {
			for (auto& subpath : parts) {
				currentNode->containsDirectories = true;
				// TODO: What a mess, simplify
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

/*
if (ImGui::TreeNode("Basic trees")) {
	for (int i = 0; i < 5; i++) {
		// Use SetNextItemOpen() so set the default state of a node to be open.
		// We could also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same
		thing !if (i == 0) ImGui::SetNextItemOpen(true, ImGuiCond_Once);

		if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i)) {
			ImGui::Text("blah blah");
			ImGui::SameLine();
			if (ImGui::SmallButton("button")) {
			};
			ImGui::TreePop();
		}
	}
	ImGui::TreePop();
}
*/


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

namespace {
	void MaybeItemHover(PodEntry* entry)
	{
		if (ImGui::IsItemHovered()) {
			ImGui::PushFont(ImguiImpl::s_CodeFont);
			std::string text = fmt::format("Path:\t{}\nName:\t{}\nType:\t{}\n Ptr:\t{}\n UID:\t{}", entry->path,
				entry->name, entry->type.name(), entry->ptr, entry->uid);

			if (!entry->ptr) {
				ImUtil::TextTooltipUtil(text);
				ImGui::PopFont();
				return;
			}

			text += "\n";
			text += "\n";
			text += "\n";

			text += refltools::PropertiesToText(entry->ptr.get());

			ImUtil::TextTooltipUtil(text);
			ImGui::PopFont();
		}
	}
} // namespace

void AssetsWindow::DrawAsset(PodEntry* assetEntry)
{
	ImGui::PushID(static_cast<int32>(assetEntry->uid));
	bool disabled = !(assetEntry->ptr);

	if (disabled) {
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.6f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.7f, 0.6f));
		if (ImGui::Button("Reload")) {
		}
		ImGui::PopStyleColor(3);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
	}
	else {
	}

	ImGui::SameLine();
	ImGui::Text(assetEntry->path.c_str());
	if (disabled) {
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
	MaybeItemHover(assetEntry);
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
	// TODO: Disable this button
	if (ImEd::Button("Back") && CanBack) {
		m_currentPath.erase(prevDirPos);
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
} // namespace ed
