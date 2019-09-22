#include "pch.h"
#include "imgui/imgui.h"
#include "editor/AssetWindow.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"

void AssetWindow::Init()
{
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".gltf", m_gltf);
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".xscn", m_xscn);

	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".jpg", m_images);
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".png", m_images);
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".tga", m_images);
}

void AssetWindow::Draw()
{
	ImGui::Begin("Asset Library");
	int32 n = 0;

	if (ImGui::CollapsingHeader("Model files"))
	{
		for (auto& s : m_gltf)
		{
			DrawAsset(n, s);
		}
	}

	if (ImGui::CollapsingHeader("Image files"))
	{
		for (auto& s : m_images)
		{
			DrawAsset(n, s);
		}
	}

	if (ImGui::CollapsingHeader("Scene files"))
	{
		for (auto& s : m_xscn)
		{
			DrawAsset(n, s);
		}
	}
	ImGui::End();
}


// Our buttons are both drag sources and drag targets here!


void AssetWindow::DrawAsset(int32& n, const std::string& path)
{
	ImGui::PushID(n++);
	ImGui::Button(path.c_str());

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("ASSET_PATH", &path, sizeof(std::string));
		ImGui::EndDragDropSource();
	}
	ImGui::PopID();
}
