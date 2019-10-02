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
#include "asset/PodIncludes.h"

#include <iostream>
#include "asset/AssetManager.h"
#include "core/reflection/ReflectionTools.h"

#include <set>

namespace {

// Recursively adds all children too
void ImGuiNode(Node* node, int32 depth, Node*& selectedNode) {
	auto str = std::string(depth * 4, ' ') + node->GetClass().GetNameStr();
	
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

/*
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
	
	Node* node;

	bool dirty{ false };

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
	
	template<typename T>
	void Visit(T& t, ExactProperty& p)
	{
		if (Inner(t, p))
		{
			dirty = true;
		}
	}

	bool Inner(int32& t, ExactProperty& p)
	{
		return ImGui::DragInt(name, &t, 0.1f);
	}

	bool Inner(bool& t, ExactProperty& p)
	{
		if (p.HasFlags(NoEdit))
		{
			bool t1 = t;
			ImGui::Checkbox(name, &t1);
			return false;
		}
		return ImGui::Checkbox(name, &t);
	}
	
	bool Inner(float& t, ExactProperty& p)
	{
		return ImGui::DragFloat(name, &t, 0.01f);
	}

	bool Inner(glm::vec3& t, ExactProperty& p)
	{
		if (p.HasFlags(PropertyFlags::Color))
		{
			return ImGui::ColorEdit3(name, ImUtil::FromVec3(t), ImGuiColorEditFlags_DisplayHSV);
		}
		else 
		{
			return ImGui::DragFloat3(name, ImUtil::FromVec3(t), 0.01f);
		}
	}

	bool Inner(glm::vec4& t, ExactProperty& p)
	{
		if (p.HasFlags(PropertyFlags::Color))
		{
			return ImGui::ColorEdit4(name, ImUtil::FromVec4(t), ImGuiColorEditFlags_DisplayHSV);
		}
		else
		{
			return ImGui::DragFloat4(name, ImUtil::FromVec4(t), 0.01f);
		}
	}

	bool Inner(std::string& ref, ExactProperty& p) 
	{
		if (p.HasFlags(PropertyFlags::Multiline))
		{
			return ImGui::InputTextMultiline(name, &ref, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
		}
		else
		{
			return ImGui::InputText(name, &ref);
		}
	}

	template<typename PodType>
	void PodDropTarget(PodHandle<PodType>& pod)
	{
//		std::string payloadTag = "POD_UID_" + std::to_string(pod->type.hash());

		if (ImGui::BeginDragDropTarget())
		{
			std::string payloadTag = "POD_UID_" + std::to_string(ctti::type_id<PodType>().hash());
			if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload(payloadTag.c_str()))
			{
				assert(payload->DataSize == sizeof(size_t));
				size_t uid = *reinterpret_cast<size_t*>(payload->Data);
				pod.podId = uid;
				//pod = AssetManager::GetOrCreate<PodType>(fs::path(*payloadStr) / "#model");
				Engine::GetRenderer<EditorRenderer>()->OnNodePodsDirty(node);
			}
			ImGui::EndDragDropTarget();
		}
	}

	template<typename PodType>
	bool Inner(PodHandle<PodType>& pod, ExactProperty& p)
	{
		if (!pod.HasBeenAssigned())
		{
			std::string s = "Unitialised handle: " + p.GetName();
			ImGui::Text(s.c_str());
			return false;
		}

		auto str = Engine::GetAssetManager()->GetPodPath(pod).string();
		bool open = ImGui::CollapsingHeader(name);
		PodDropTarget(pod);

		if (open)
		{
			GenerateUniqueName(p);
			ImGui::InputText(name, &str, ImGuiInputTextFlags_ReadOnly);
			PodDropTarget(pod);
		

			depth++;
			ImGui::Indent();
			
			CallVisitorOnEveryProperty(pod.operator->(), *this);

			ImGui::Unindent();
			depth--;
		}
		return false;
	}

	template<typename T>
	bool Inner(T& t, ExactProperty& p)
	{
		std::string s = "unhandled property: " + p.GetName();
		ImGui::Text(s.c_str());
		return false;
	}

	template<typename T>
	bool Inner(std::vector<PodHandle<T>>& t, ExactProperty& p)
	{
		if (ImGui::CollapsingHeader(name))
		{
			ImGui::Indent();
			int32 index = 0;
			for (auto& handle : t)
			{
				++index;
				std::string sname = "|" + p.GetName() + std::to_string(index);
				size_t len = sname.size();
				path += sname;

				GenerateUniqueName(p);
				std::string finalName = Engine::GetAssetManager()->GetPodPath(handle).string() + "##" + name;
				bool r = ImGui::CollapsingHeader(finalName.c_str());
				PodDropTarget(handle);
				if (r)
				{
					ImGui::Indent();
					CallVisitorOnEveryProperty(handle.operator->(), *this);
					ImGui::Unindent();
				}

				path.erase(path.end() - (len), path.end());
			}
			ImGui::Unindent();
		}
		return false;
	}
};


}
*/

Editor::Editor()
	: m_selectedNode(nullptr)
	, m_updateWorld(true)
{
	ImguiImpl::InitContext();
	m_assetWindow.Init();
}

Editor::~Editor()
{
	ImguiImpl::CleanupContext();
}
#include "editor/renderer/EditorRenderer.h"
#include "system/Input.h"
	
void Editor::UpdateEditor()
{
	HandleInput();


	if (!m_showImgui)
	{
		return;
	}

	ImguiImpl::NewFrame();

	static ImGui::FileBrowser lfb = ImGui::FileBrowser(
		ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);

	ImGui::ShowDemoWindow();
		
	static bool open = true;

	ImGui::Begin("Editor", &open);
	ImGui::Checkbox("Update World", &m_updateWorld);

	if (ImGui::Button("Save"))
	{
		m_sceneSave.OpenBrowser();
	}

	m_sceneSave.Draw();
	ImGui::SameLine();
	
	if (ImGui::Button("Load"))
	{
		lfb.SetTitle("Load World"); 
		lfb.Open();
	}

	auto renderer = Engine::GetRenderer<EditorRenderer>();
	if (renderer)
	{
		const char* str = renderer->m_previewModeString.c_str();
		ImGui::Text(str);
	}
	

	lfb.Display();

	if (lfb.HasSelected()) 
	{
		m_sceneToLoad = lfb.GetSelected().string();
		lfb.ClearSelected();
	}


	/*static std::string model;
	ImGui::InputText("ModelAsset to load:", &model);
	ImGui::SameLine();
	if (ImGui::Button("Create Asset"))
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
		if (ImGui::CollapsingHeader(refl::GetClass(m_selectedNode).GetNameStr().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
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
			if (ImGui::Button("Unload"))
			{
				AssetManager::ClearFromId(assetPair.second);
			}
			ImGui::SameLine();
			ImGui::Text(assetPair.first.c_str());
			ImGui::SameLine();
			ImGui::Text(std::to_string(assetPair.second).c_str());
			
			//text += assetPair.first + "\n";
		}
	}

	ImGui::End();


	m_assetWindow.Draw();


	ImguiImpl::EndFrame();
}

void Editor::Outliner()
{
	ImGui::BeginChild("Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);


	RecurseNodes(Engine::GetWorld()->GetRoot(), [&](Node* node, int32 depth)
	{
		auto str = std::string(depth * 4, ' ') + node->m_type + "> " + node->m_name + "##" + node->GetClass().GetNameStr();
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
	ImGui::InputText("Name", &node->m_name);

	if (ImGui::Button("Serialize-Save"))
	{
		SceneSave::SerializeNodeData(node);
	}


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

	
	if (ImGui::Button("Duplicate"))
	{
		Node* newnode = Engine::GetWorld()->DeepDuplicateNode(node);
		m_selectedNode = newnode;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete"))
	{
		Engine::GetWorld()->DeleteNode(node);
		m_selectedNode = nullptr;
		ImGui::EndChild();
		return;
	}


	auto camera = dynamic_cast<CameraNode*>(node);
	if (camera)
	{
		ImGui::SameLine();
		if (ImGui::Button("Make Active Camera"))
		{
			Engine::GetWorld()->m_activeCamera = camera;
		}
	}

	ImGui::Separator();
	//ReflectionToImguiVisitor visitor;
	//visitor.node = node;
	//CallVisitorOnEveryProperty(node,visitor);

	ImGui::EndChild();

	//if (visitor.dirty)
	{
		node->MarkDirty();
	}

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

void Editor::HandleInput()
{
	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::TAB))
	{
		m_showImgui = !m_showImgui;
	}

	if (Engine::GetInput()->IsKeyRepeat(XVirtualKey::CTRL) && Engine::GetInput()->IsKeyPressed(XVirtualKey::W))
	{
		if (m_selectedNode && m_selectedNode != Engine::GetWorld()->GetRoot())
		{
			Engine::GetWorld()->DeepDuplicateNode(m_selectedNode);
		}
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

