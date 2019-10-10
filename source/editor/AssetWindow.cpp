#include "pch/pch.h"
#include "imgui/imgui.h"
#include "editor/AssetWindow.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "core/reflection/ReflectionTools.h"
#include "editor/imgui/ImguiExtensions.h"
#include "core/reflection/PodTools.h"


void AssetWindow::Init()
{
	Timer::DebugTimer<std::chrono::milliseconds> timer(true);

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

	if (ImGui::CollapsingHeader("Model files")) {
		ImGui::Indent();
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
		ImGui::Unindent();
	}
}

void AssetWindow::Draw()
{
	ImGui::Begin("Asset Window");
	DrawFileLibrary();
	ImGui::End();
}

#include "asset/UriLibrary.h"

void AssetWindow::DrawFileAsset(int32& n, const std::string& zpath)
{
}
