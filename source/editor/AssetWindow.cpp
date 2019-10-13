#include "pch/pch.h"

#include "editor/AssetWindow.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "reflection/ReflectionTools.h"
#include "reflection/PodTools.h"

#include <imgui/imgui.h>

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
			auto strPath = "/" + s.second.string();
			for (int32 i = 0; i < strPath.size(); ++i) {
				if (strPath[i] == '\\') {
					strPath[i] = '/';
				}
			}

			ImGui::Button(s.first.c_str());

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
				auto h = AssetManager::GetOrCreate<ModelPod>(strPath);

				std::string payloadTag = "POD_UID_" + std::to_string(h->type.hash());
				ImGui::SetDragDropPayload(payloadTag.c_str(), &h.podId, sizeof(size_t));
				ImGui::EndDragDropSource();
			}
		}

		ImGui::PopID();
	}
}

void AssetWindow::Draw()
{
	ImGui::Begin("Model Files");
	if (ImGui::CollapsingHeader("Model Files")) {
		ImGui::Indent();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
		if (ImGui::Button("Refresh Files") || m_needsRefresh) {
			ReloadCache();
			m_needsRefresh = false;
		}
		ImGui::PopStyleVar();

		ImGui::Unindent();
		ImGui::Separator();

		DrawFileLibrary();
	}

	ImGui::End();
}
