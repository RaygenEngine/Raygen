#include "pch.h"
#include "imgui/imgui.h"
#include "editor/Editor.h"
#include "world/World.h"
#include "system/Engine.h"
#include "editor/imgui/ImguiExtensions.h"
#include "editor/imgui/ImguiImpl.h"


namespace Editor
{
using World::Node;

	namespace {
	// Recursively adds all children too
	void ImGuiNode(Node* node, int32 depth, Node*& selectedNode) {
		auto str = std::string(depth * 4, ' ') + node->m_reflector.GetName();
	
		if (ImGui::Selectable(str.c_str(), node == selectedNode))
		{
			selectedNode = node;
		}
	
		for (auto c : node->GetChildren())
		{
			ImGuiNode(c.get(), depth + 1, selectedNode);
		}
	}
	}

	Editor::Editor(System::Engine* engine)
		: EngineObject(engine)
		, m_selectedNode(nullptr)
		, m_updateWorld(true)
	{
		// imgui init would be called here if HWND was accessible
		// ImguiImpl::Init();
	}
	
	void Editor::UpdateEditor()
	{
		ImguiImpl::NewFrame();

		ImGui::ShowDemoWindow();
		
		static bool open = true;
		ImGui::Begin("Editor", &open);
		ImGui::Checkbox("Update World", &m_updateWorld);
		if (ImGui::CollapsingHeader("Outliner", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Outliner();
		}

		if (m_selectedNode) {
			if (ImGui::CollapsingHeader(GetReflector(m_selectedNode).GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				PropertyEditor(m_selectedNode);
			}
		}

		ImGui::End();

		ImguiImpl::EndFrame();
	}

	void Editor::Outliner()
	{
		ImGui::BeginChild("Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGuiNode(GetWorld(), 0, m_selectedNode);
		ImGui::EndChild();
	}

	void Editor::PropertyEditor(Node* node)
	{
		ImGui::BeginChild("Properties", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	
		glm::vec3 eulerPyr = glm::degrees(glm::eulerAngles(node->m_localOrientation));

		bool dirtyMatrix = false;

		if (ImGui::DragFloat3("Position", ImUtil::FromVec3(node->m_localTranslation), 0.01f))
		{
			dirtyMatrix = true;
		}
		if (ImGui::DragFloat3("Rotation", ImUtil::FromVec3(eulerPyr), 0.1f))
		{
			node->m_localOrientation = glm::quat(glm::radians(eulerPyr));
			dirtyMatrix = true;
		}
		if (ImGui::DragFloat3("Scale", ImUtil::FromVec3(node->m_localScale), 0.01f))
		{
			dirtyMatrix = true;
		}
		ImGui::Separator();
		bool dirty = ImUtil::AddReflector(node->m_reflector);
		ImGui::EndChild();

		if (dirtyMatrix)
		{
			node->m_localMatrix = Core::GetTransformMat(node->m_localTranslation, node->m_localOrientation, node->m_localScale);
		}
		if (dirty || dirtyMatrix) 
		{
			node->MarkDirty();
		}
	}

}
