#include "pch.h"
#include "imgui/imgui.h"
#include "editor/Editor.h"
#include "world/World.h"
#include "system/Engine.h"
#include "editor/imgui/ImguiExtensions.h"
#include "editor/imgui/ImguiImpl.h"
#include "tinyxml2/tinyxml2.h"
#include "imgui_ext/imfilebrowser.h"

namespace Editor
{
using World::Node;

	namespace {


	template<typename Lambda>
	void RecurseNodes(Node* root, Lambda f, int32 depth = 0)
	{
		if (!root) 
		{
			return;
		}

		f(root, depth);
		for (auto c : root->GetChildren())
		{
			RecurseNodes(c.get(), f, depth + 1);
		}
	}



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

		// TODO: static fix this
		static ImGui::FileBrowser fb;



		ImGui::ShowDemoWindow();
		
		static bool open = true;

		ImGui::Begin("Editor", &open);
		ImGui::Checkbox("Update World", &m_updateWorld);
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			fb.Open();
		}

		fb.Display();

		if (fb.HasSelected())
		{
			SaveScene(fb.GetSelected().string());
			fb.ClearSelected();
		}

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


		RecurseNodes(GetWorld(), [&](Node* node, int32 depth)
		{
			auto str = std::string(depth * 4, ' ') + node->m_reflector.GetName();
			if (ImGui::Selectable(str.c_str(), node == m_selectedNode))
			{
				m_selectedNode = node;
			}
		});
		ImGui::EndChild();
	}

	namespace
	{

		bool AddReflector(Reflector& reflector)
		{
			bool dirty = false;
			for (auto& prop : reflector.GetProperties())
			{
				auto str = prop.GetName().c_str();

				dirty |= prop.SwitchOnType(
					[&str](int& ref) {
					return ImGui::DragInt(str, &ref, 0.1f);
				},
					[&str](bool& ref) {
					return ImGui::Checkbox(str, &ref);
				},
					[&str](float& ref) {
					return ImGui::DragFloat(str, &ref, 0.01f);
				},
					[&str, &prop](glm::vec3& ref) {
					if (prop.HasFlags(PropertyFlags::Color))
					{
						return ImGui::ColorEdit3(str, ImUtil::FromVec3(ref), ImGuiColorEditFlags_DisplayHSV);
					}
					return ImGui::DragFloat3(str, ImUtil::FromVec3(ref), 0.01f);
				},
					[&str](std::string& ref) {
					return ImGui::InputText(str, &ref);
				});
			}
			return dirty;
		}

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
		bool dirty = AddReflector(node->m_reflector);
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
	
	
	namespace
	{
		tinyxml2::XMLElement* GenerateNodeXML(Node* node, tinyxml2::XMLDocument& document)
		{
			tinyxml2::XMLElement* xmlElem;

			xmlElem = document.NewElement(node->GetType().c_str());
			xmlElem->SetAttribute("name", node->GetName().c_str());

			for (auto& p : GetReflector(node).GetProperties())
			{
				if (!p.HasFlags(PropertyFlags::NoSave))
				{
					p.SwitchOnType(
						[&](int32& v) {
						xmlElem->SetAttribute(p.GetName().c_str(), v);
					},
						[&](bool& v) {
						xmlElem->SetAttribute(p.GetName().c_str(), v);
					},
						[&](float& v) {
						xmlElem->SetAttribute(p.GetName().c_str(), v);
					},
						[&](glm::vec3& v) {
						xmlElem->SetAttribute(p.GetName().c_str(), Assets::FloatsToString(v).c_str());
					},
						[&](std::string& v) {
						xmlElem->SetAttribute(p.GetName().c_str(), v.c_str());
					});
				}
			}
			return xmlElem;
		}
	}

	void Editor::SaveScene(const std::string& filename)
	{

		using namespace tinyxml2;
		XMLDocument xmlDoc;
	
		std::unordered_map<Node*, XMLElement*> nodeXmlElements;

		auto root = GenerateNodeXML(GetWorld(), xmlDoc);
		xmlDoc.InsertFirstChild(root);
		
		nodeXmlElements.insert({ GetWorld(), root });


		RecurseNodes(GetWorld(), [&](Node* node, auto)
		{
			if (node == GetWorld())
			{
				return;
			}
			
			auto xmlElem = GenerateNodeXML(node, xmlDoc);
			nodeXmlElements.insert({ node, xmlElem });

			Node* parent = dynamic_cast<Node*>(node->GetParentObject());
			nodeXmlElements[parent]->InsertEndChild(xmlElem);
		});

		xmlDoc.Print();
	}

}
