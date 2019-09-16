#include "pch.h"
#include "imgui/imgui.h"
#include "editor/Editor.h"
#include "world/World.h"
#include "system/Engine.h"
#include "editor/imgui/ImguiExtensions.h"
#include "editor/imgui/ImguiImpl.h"
#include "tinyxml2/tinyxml2.h"
#include "imgui_ext/imfilebrowser.h"
#include "platform/windows/Win32Window.h"
#include "system/EngineEvents.h"

#include <filesystem>

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


namespace
{
bool AddReflector(Reflector& reflector, int32 depth = 0)
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
		}
			//		,[&str, depth](Asset*& ref) {
			//			if (ImGui::CollapsingHeader((std::string(str) + "##" + std::to_string(depth)).c_str()))
			//			{
			//				ImGui::Indent();
			//				std::string s = "No Asset";
			//				if (ref)
			//				{
			//					s = fs::relative(ref->GetUri()).string();//.string();
			//				}
			//
			//				if (ImGui::Button("Deallocate"))
			//				{
			////					Engine::GetAssetManager()->Unload(ref);
			//				}
			//				ImGui::SameLine();
			//
			//				if (ImGui::Button("Reload"))
			//				{
			//	//				Engine::GetAssetManager()->Load(ref);
			//				}
			//				ImGui::SameLine();
			//				ImGui::InputText("", &s);
			//
			//				if (ref)
			//				{
			//					ImGui::Indent();
			//					AddReflector(GetReflector(ref), depth + 1);
			//				}
			//			}
			//			return true;
			//		}
		);
	}
	return dirty;
}
}


Editor::Editor()
	: m_selectedNode(nullptr)
	, m_updateWorld(true)
{
	ImguiImpl::InitContext();
}

Editor::~Editor()
{
	ImguiImpl::CleanupContext();
}

struct ReflStruct
{
	STATIC_REFLECTOR(ReflStruct)
	{
		S_REFLECT_VAR(number);
		S_REFLECT_VAR(v, PropertyFlags::Color);
	}

	int32 number;
	glm::vec3 v;
};
	
void Editor::UpdateEditor()
{
	ImguiImpl::NewFrame();



	// TODO: static fix this
	static ImGui::FileBrowser sfb = ImGui::FileBrowser(
		ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_EnterNewFilename
		| ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CreateNewDir
		| ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);

	static ImGui::FileBrowser lfb = ImGui::FileBrowser(
		ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);

	ImGui::ShowDemoWindow();
		
	static bool open = true;

	ImGui::Begin("Editor", &open);
	ImGui::Checkbox("Update World", &m_updateWorld);
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		sfb.SetTitle("Save World"); 
		sfb.Open();
	}
	ImGui::SameLine();
	
	if (ImGui::Button("Load"))
	{
		lfb.SetTitle("Load World"); 
		lfb.Open();
	}

	sfb.Display();
	lfb.Display();


	if (sfb.HasSelected())
	{
		SaveScene(sfb.GetSelected().string());
		sfb.ClearSelected();
	}

	if (lfb.HasSelected()) 
	{
		m_sceneToLoad = lfb.GetSelected().string();
		lfb.ClearSelected();
	}


	static std::string model;
	ImGui::InputText("ModelAsset to load:", &model);
	ImGui::SameLine();
	/*if (ImGui::Button("Create Asset"))
	{
		auto added =  Engine::GetWorld()->LoadNode<TriangleModelGeometryNode>(Engine::GetWorld()->GetRoot());
		
		auto path = Engine::GetAssetManager()->m_pathSystem.SearchAssetPath(model);
		auto asset = Engine::GetAssetManager()->RequestAsset<ModelAsset>(path / "model");
		GetReflector(added).GetPropertyByName("model")->GetRef<ModelAsset*>() = asset;
		Engine::GetAssetManager()->Load(asset);
		
		Event::OnWorldNodeAdded.Broadcast(added);
	}*/

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



	if (ImGui::CollapsingHeader("Assets"))
	{
		ImGui::Indent();
		std::string text;
		for (auto& assetPair : Engine::GetAssetManager()->m_pathToUid)
		{
			ImGui::Text(assetPair.first.c_str());
			ImGui::SameLine();
			ImGui::Text(std::to_string(assetPair.second).c_str());

			//text += assetPair.first + "\n";
		}
	}



	ImGui::End();

	ImguiImpl::EndFrame();
}

void Editor::Outliner()
{
	ImGui::BeginChild("Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);


	RecurseNodes(Engine::GetWorld()->GetRoot(), [&](Node* node, int32 depth)
	{
		auto str = std::string(depth * 4, ' ') + node->m_reflector.GetName();
		if (ImGui::Selectable(str.c_str(), node == m_selectedNode))
		{
			m_selectedNode = node;
		}
	});
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

	
	if (ImGui::DragFloat3("AbsScale", ImUtil::FromVec3(node->m_worldScale), 0.01f))
	{
		dirtyMatrix = true;
	}

	ImGui::Separator();
	bool dirty = AddReflector(node->m_reflector);
	ImGui::EndChild();

	if (dirtyMatrix)
	{
		node->m_localMatrix = utl::GetTransformMat(node->m_localTranslation, node->m_localOrientation, node->m_localScale);
	}
	if (dirty || dirtyMatrix) 
	{
		node->MarkDirty();
	}
}

void Editor::LoadScene(const std::string& scenefile)
{
	Engine::Get().CreateWorldFromFile(scenefile);
	Engine::Get().SwitchRenderer(0);

	m_selectedNode = nullptr;
	Event::OnWindowResize.Broadcast(Engine::GetMainWindow()->GetWidth(), Engine::GetMainWindow()->GetHeight());
}
	
namespace
{
	tinyxml2::XMLElement* GenerateNodeXML(Node* node, tinyxml2::XMLDocument& document)
	{
		tinyxml2::XMLElement* xmlElem;

		xmlElem = document.NewElement(node->GetType().c_str());
		xmlElem->SetAttribute("name", node->GetName().c_str());

		xmlElem->SetAttribute("translation", ParsingAux::FloatsToString(node->GetLocalTranslation()).c_str());
		xmlElem->SetAttribute("euler_pyr", ParsingAux::FloatsToString(node->GetLocalPYR()).c_str());
		xmlElem->SetAttribute("scale", ParsingAux::FloatsToString(node->GetLocalScale()).c_str());

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
					xmlElem->SetAttribute(p.GetName().c_str(), ParsingAux::FloatsToString(v).c_str());
				},
					[&](std::string& v) {
					xmlElem->SetAttribute(p.GetName().c_str(), v.c_str());
				}
				//	,[&](Asset*& v) {
				//	if (v)
				//	{
				//		std::string assetFile;
				//		assetFile = v->GetUri().string();
				//		xmlElem->SetAttribute(p.GetName().c_str(), assetFile.c_str());
				//	}
				//}
				);
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

	auto root = Engine::GetWorld()->GetRoot();
	auto rootXml = GenerateNodeXML(root, xmlDoc);
	xmlDoc.InsertFirstChild(rootXml);

	nodeXmlElements.insert({ root, rootXml });


	RecurseNodes(root, [&](Node* node, auto)
	{
		if (node == root)
		{
			return;
		}

		auto xmlElem = GenerateNodeXML(node, xmlDoc);
		nodeXmlElements.insert({ node, xmlElem });

		Node* parent = node->GetParent();
		nodeXmlElements[parent]->InsertEndChild(xmlElem);
	});

		

	FILE* fp;
	if (fopen_s(&fp, filename.c_str(), "w") == 0)
	{
		xmlDoc.SaveFile(fp);
		fclose(fp);
	}
	else
	{
		LOG_ERROR("Failed to open file for saving scene.");
	}
}

void Editor::PreBeginFrame()
{
	if (m_sceneToLoad.size() > 0)
	{
		LoadScene(m_sceneToLoad);
		m_sceneToLoad = "";
	}
}

