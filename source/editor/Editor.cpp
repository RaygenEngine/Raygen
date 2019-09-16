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
#include "asset/pods/CubemapPod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/pods/ImagePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/pods/ModelPod.h"
#include "asset/pods/ShaderPod.h"
#include "asset/pods/TextPod.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/XMLDocPod.h"


#include <iostream>
#include "asset/AssetManager.h"
#include "system/reflection/ReflectionTools.h"

#include <set>

struct ReflVisitor
{
	int32 depth{ 0 };

	template<typename T>
	void Visit(T& t, ExactProperty& p)
	{
		std::cout << std::string("\t", depth);
		std::cout << "| " << p.GetName() << " -> " << t << std::endl;
	}

	void Visit(glm::vec3& t, ExactProperty& p)
	{
		std::cout << std::string("\t", depth);
		std::cout << "| " << p.GetName() << " -> " << t.x << ", " << t.y << ", " << t.z << std::endl;
	}

	template<typename T>
	void Visit(PodHandle<T>& t, ExactProperty& p)
	{
		std::cout << std::string("\t", depth);
		std::cout << "Visited by pod: " << p.GetName() << "@" << Engine::GetAssetManager()->GetPodPath(t) << std::endl;
		depth++;
		CallVisitorOnEveryProperty(t.operator->(), *this);
		depth--;
	}

	
	template<typename T>
	void Visit(std::vector<T>& t, ExactProperty& p)
	{
		std::cout << "Vector of anything" << std::endl;
	}
};

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
using namespace PropertyFlags;

struct ReflectionToImguiVisitor : public ReflectionTools::Example
{
	int32 depth{ 0 };
	std::string path{};
	
	std::set<std::string> objNames;

	std::string nameBuf;
	const char* name;  

	void GenerateUniqueName(ExactProperty& p)
	{
		std::string buf = p.GetName() + "##" + path;
		auto r = objNames.insert(buf);
		int32 index = 0;
		while (r.second == false)
		{
			r = objNames.insert(buf + std::to_string(index));
			index++;
		}
		name = r.first->c_str();
	}

	void Begin(Reflector& r)
	{
		path += "|" + r.GetName();
	}

	void End(Reflector& r)
	{
		path.erase(path.end() - (r.GetName().size() + 1), path.end());
	}

	void PreProperty(ExactProperty& p)
	{
		GenerateUniqueName(p);
	}
	
	void Visit(int32& t, ExactProperty& p)
	{
		ImGui::DragInt(name, &t, 0.1f);
	}

	void Visit(bool& t, ExactProperty& p)
	{
		if (p.HasFlags(NoEdit))
		{
			bool t1 = t;
			ImGui::Checkbox(name, &t1);
		}
		else
		{
			ImGui::Checkbox(name, &t);
		}
	}
	
	void Visit(float& t, ExactProperty& p)
	{
		ImGui::DragFloat(name, &t, 0.01f);
	}

	void Visit(glm::vec3& t, ExactProperty& p)
	{
		if (p.HasFlags(PropertyFlags::Color))
		{
			ImGui::ColorEdit3(name, ImUtil::FromVec3(t), ImGuiColorEditFlags_DisplayHSV);
		}
		else 
		{
			ImGui::DragFloat3(name, ImUtil::FromVec3(t), 0.01f);
		}
	}

	void Visit(std::string& ref, ExactProperty& p) 
	{
		if (p.HasFlags(PropertyFlags::Multiline))
		{
			ImGui::InputTextMultiline(name, &ref, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
		}
		else
		{
			ImGui::InputText(name, &ref);
		}
	}

	template<typename PodType>
	void Visit(PodHandle<PodType>& pod, ExactProperty& p)
	{
		if (!pod.HasBeenAssigned())
		{
			std::string s = "Unitialised handle: " + p.GetName();
			ImGui::Text(s.c_str());
			return;
		}

		auto str = Engine::GetAssetManager()->GetPodPath(pod).string();
		if (ImGui::CollapsingHeader(name))
		{
			GenerateUniqueName(p);
			ImGui::InputText(name, &str, ImGuiInputTextFlags_ReadOnly);
			depth++;
			ImGui::Indent();
			
			CallVisitorOnEveryProperty(pod.operator->(), *this);

			ImGui::Unindent();
			depth--;
		}
	}


	template<typename T>
	void Visit(T& t, ExactProperty& p)
	{
		std::string s = "unhandled property: " + p.GetName();
		ImGui::Text(s.c_str());
	}


};


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
	
	CallVisitorOnEveryProperty(node, ReflectionToImguiVisitor());

	ImGui::EndChild();

	if (dirtyMatrix)
	{
		node->m_localMatrix = utl::GetTransformMat(node->m_localTranslation, node->m_localOrientation, node->m_localScale);
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

