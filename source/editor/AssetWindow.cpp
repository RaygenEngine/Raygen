#include "pch/pch.h"

#include "editor/AssetWindow.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "reflection/ReflectionTools.h"
#include "reflection/PodTools.h"

#include <imgui/imgui.h>
#include "editor/imgui/ImguiUtil.h"
#include "editor/DataStrings.h"
#include "editor/Editor.h"

void AssetWindow::ReloadCache()
{
	m_gltf.clear();
	timer::DebugTimer<std::chrono::milliseconds> timer(true);

	for (const auto& entry : fs::recursive_directory_iterator(fs::current_path())) {
		if (entry.is_directory()) {
			continue;
		}

		if (entry.path().extension() == ".gltf") {
			auto key = entry.path().filename().string();

			auto& pathEntry = m_gltf[key];
			if (pathEntry.empty()) {
				pathEntry = std::filesystem::relative(entry);
			}
		}
	}

	LOG_INFO("Cached {} gltf files in {} ms.", m_gltf.size(), timer.Get());
}

void AssetWindow::DrawFileLibrary()
{
	int32 n = 0;

	for (auto& s : m_gltf) {
		ImGui::PushID(n++);
		if (s.first.size() > 2) {
			if (!m_filter.PassFilter(s.first.c_str())) {
				ImGui::PopID();
				continue;
			}

			ImGui::Button(s.first.c_str());

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
				auto strPath = "/" + s.second.string();
				for (int32 i = 0; i < strPath.size(); ++i) {
					if (strPath[i] == '\\') {
						strPath[i] = '/';
					}
				}

				auto h = AssetManager::GetOrCreate<ModelPod>(strPath);

				std::string payloadTag = "POD_UID_" + std::to_string(h.Lock()->type.hash());
				ImGui::SetDragDropPayload(payloadTag.c_str(), &h.podId, sizeof(size_t));
				ImGui::EndDragDropSource();
			}
			TEXT_TOOLTIP("Drag this onto the outliner to create a new geometry node with this gltf model.");
		}

		ImGui::PopID();
	}
}

bool AssetWindow::Draw()
{
	bool result = true;
	ImGui::SetNextWindowPos(ImVec2(500, 250), ImGuiCond_FirstUseEver);
	// Attempt to predict the viewport size for the first run, might be a bit off.
	ImGui::SetNextWindowSize(ImVec2(250, 600), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Model Files", &result)) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
		if (ImGui::Button("Refresh") || m_needsRefresh) {
			ReloadCache();
			m_needsRefresh = false;
		}
		Editor::HelpTooltipInline(help_GltfWindowRefresh);

		ImGui::SameLine();
		m_filter.Draw("Search", ImGui::GetFontSize() * 8);

		ImGui::PopStyleVar();

		ImGui::Separator();

		DrawFileLibrary();
	}

	ImGui::End();
	return result;
}
