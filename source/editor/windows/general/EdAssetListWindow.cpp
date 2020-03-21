#include "pch.h"
#include "editor/windows/general/EdAssetListWindow.h"
#include "asset/AssetManager.h"
#include "core/StringUtl.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiUtil.h"
#include "reflection/ReflectionTools.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/Editor.h"
#include "editor/utl/EdAssetUtils.h"

#include <spdlog/fmt/fmt.h>
#include <imgui_internal.h>

namespace ed {
AssetListWindow::AssetListWindow(std::string_view name)
	: UniqueWindow(name)
{
}

void AssetListWindow::ImguiDraw()
{
	static ImGuiTextFilter filter;
	ImGui::Text(U8(FA_SEARCH));
	ImGui::SameLine();
	filter.Draw("###SearchFilter", ImGui::GetFontSize() * 14);

	ImGui::SameLine();
	if (ImGui::Button(ETXT(FA_FILTER, "Filter"))) {
		ImGui::OpenPopup("###AssetListWindowFilterContext");
	}

	if (ImGui::BeginPopup("###AssetListWindowFilterContext")) {
		ImGui::Checkbox("Unsaved", &m_showUnsaved);
		ImGui::Checkbox("Saved", &m_showSaved);
		ImGui::Checkbox("Transient", &m_showTransient);
		ImGui::EndPopup();
	}


	bool saveAll = ImEd::Button(U8(FA_SAVE u8"  Save All"));
	ImGui::SameLine();

	bool reloadAll = ImEd::Button(U8(FA_REFRESH_A u8"  Reload All"));
	ImGui::SameLine();

	bool reimportAll = ImEd::Button(U8(FA_FILE_IMPORT u8"  Reimport All"));

	if (!ImGui::CollapsingHeader("Asset List")) {
		return;
	}

	ImGui::Dummy({ 0.f, 3.f });
	ImGui::Separator();
	ImGui::Dummy({ 0.f, 3.f });


	if (!ImGui::BeginChild("AssetList_SubWindow_List")) {
		return;
	}
	auto& entries = AssetHandlerManager::Z_GetPods();
	for (auto& entryPtr : entries) {
		auto& entry = *entryPtr.get();

		if (filter.IsActive() && !filter.PassFilter(entry.path.c_str()) && !filter.PassFilter(entry.name.c_str())
			&& !filter.PassFilter(entry.metadata.originalImportLocation.c_str())
			&& !filter.PassFilter(entry.type.name_str().c_str())) {
			continue;
		}

		if (!m_showSaved && !entry.requiresSave) {
			continue;
		}
		if (!m_showUnsaved && entry.requiresSave) {
			continue;
		}

		if (!m_showTransient && entry.transient) {
			continue;
		}

		ImGui::PushID(static_cast<int>(entry.uid));
		DrawEntry(entry);
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void AssetListWindow::DrawEntry(PodEntry& entry)
{
	ImEd::DisabledSection(!entry.requiresSave, [&]() {
		if (ImGui::Button(U8(FA_SAVE))) {
		}
	});
	ImGui::SameLine();
	ImGui::Button(U8(FA_REFRESH_A));

	ImGui::SameLine();
	ImGui::Text(entry.path.c_str());
	ed::asset::MaybeHoverTooltip(&entry);
}


} // namespace ed
