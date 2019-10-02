#include "pch.h"
#include "editor/SceneSave.h"
#include "imgui/imgui.h"
#include "system/Engine.h"
#include "tinyxml2/tinyxml2.h"
#include "world/nodes/Node.h"
#include "world/World.h"
#include "system/reflection/ReflectionTools.h"
#include "asset/AssetManager.h"
#include "editor/Editor.h"

#include "asset/PodIncludes.h"
#include "asset/UriLibrary.h"

SceneSave::SceneSave()
{
	m_saveBrowser = ImGui::FileBrowser(
		ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_EnterNewFilename
		| ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CreateNewDir
		| ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc
	);
	m_saveBrowser.SetPwd();
	m_saveBrowser.SetTitle("Save Scene As");
}

void SceneSave::OpenBrowser()
{
	m_saveBrowser.Open();
}

void SceneSave::Draw()
{
	m_saveBrowser.Display();
	if (m_saveBrowser.HasSelected())
	{
		fs::path file = m_saveBrowser.GetSelected();
		m_saveBrowser.ClearSelected();
		SaveAsXML(Engine::GetWorld(), file);
	}
}

/*
namespace
{

struct GenerateXMLVisitor
{
	tinyxml2::XMLElement* xmlElement;
	const char* name;

	void operator()(int32& v, Property& p)
	{
		xmlElement->SetAttribute(p.GetNameStr().c_str(), v);
	}

	void operator()(bool& v, Property& p)
	{
		xmlElement->SetAttribute(p.GetName()., v);
	}

	void operator()(float& v, Property& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), v);
	}

	void operator()(glm::vec3& v, Property& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), ParsingAux::FloatsToString(v).c_str());
	}

	void operator()(glm::vec4& v, Property& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), ParsingAux::FloatsToString(v).c_str());
	}	
	
	void operator()(std::string& v, Property& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), v.c_str());
	}

	template<typename T>
	void operator()(PodHandle<T>& handle, Property& p)
	{
		auto podPath = Engine::GetAssetManager()->GetPodPath(handle);
		xmlElement->SetAttribute(p.GetName().c_str(), uri::RemovePathDirectory(podPath).string().c_str());
	}

	template<typename T>
	void operator()(T& v, ExactProperty& p)
	{
		LOG_WARN("Attempting to save an unimplemented save type, skipping property: ", p.GetName());
	}
};
*/

tinyxml2::XMLElement* GenerateNodeXML(Node* node, tinyxml2::XMLDocument& document)
{
	tinyxml2::XMLElement* xmlElem;

	xmlElem = document.NewElement(node->GetType().c_str());
	xmlElem->SetAttribute("name", node->GetName().c_str());

	xmlElem->SetAttribute("translation", ParsingAux::FloatsToString(node->GetLocalTranslation()).c_str());
	xmlElem->SetAttribute("euler_pyr", ParsingAux::FloatsToString(node->GetLocalPYR()).c_str());
	xmlElem->SetAttribute("scale", ParsingAux::FloatsToString(node->GetLocalScale()).c_str());
	/*
	GenerateXMLVisitor visitor;
	visitor.xmlElement = xmlElem;

	for (auto& p : GetReflector(node).GetProperties())
	{
		if (!p.HasFlags(PropertyFlags::NoSave))
		{
			CallVisitorOnProperty(p, visitor);
		}
	}
	*/
	return xmlElem;
}
//}

bool SceneSave::SaveAsXML(World* world, const fs::path& path)
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
	if (fopen_s(&fp, path.string().c_str(), "w") == 0)
	{
		xmlDoc.SaveFile(fp);
		fclose(fp);
	}
	else
	{
		LOG_ERROR("Failed to open file for saving scene.");
	}


	return false;
}

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace glm {

void to_json(json& j, const vec3& p) {
	j = json{ p[0], p[1], p[2] };
}

void to_json(json& j, const vec4& p) {
	j = json{ p[0], p[1], p[2], p[3] };
}

void from_json(const json& j, vec3& p) {
	p[0] = j.at(0);
	p[1] = j.at(1);
	p[2] = j.at(2);
}

void from_json(const json& j, vec4& p) {
	p[0] = j.at(0);
	p[1] = j.at(1);
	p[2] = j.at(2);
	p[3] = j.at(3);
}
}

template<typename T>
void to_json(json& j, const PodHandle<T>& handle)
{
	j = Engine::GetAssetManager()->GetPodPath(handle).string();
}

template<typename T>
void from_json(const json& j, PodHandle<T>& handle)
{
	handle = AssetManager::GetOrCreate(j.get<std::string>());
}


template<typename T>
json to_json_deep(const PodHandle<T>& handle)
{
	//j = Engine::GetAssetManager()->GetPodPath(handle).string();
	SerializeJsonVisitor visitor;
	CallVisitorOnEveryProperty(handle.operator->(), visitor);

	return utl::force_move(visitor.result);
}


template<typename T>
json to_json_deep(std::vector<PodHandle<T>>& vec)
{
	json result;
	for (auto& handle : vec)
	{
		SerializeJsonVisitor visitor;
		CallVisitorOnEveryProperty(handle.operator->(), visitor);
		result.emplace_back(utl::force_move(visitor.result));
	}
	return result;
}


//struct SerializeJsonVisitor
//{
//	json result;
//
//	template<typename T>
//	void Visit(T& value, ExactProperty& prop)
//	{
//		result[prop.GetName()] = value;
//	}
//
//	//template<typename T>
//	//void Visit(PodHandle<T>& value, ExactProperty& prop)
//	//{
//	//	result[prop.GetName()] = to_json_deep(value);
//	//}
//
//	//template<typename T>
//	//void Visit(std::vector<PodHandle<T>>& value, ExactProperty& prop)
//	//{
//	//	result[prop.GetName()] = to_json_deep(value);
//	//}
//};

#include <iostream>
void SceneSave::SerializeNodeData(Node* node)
{
	//SerializeJsonVisitor visitor;
	////CallVisitorOnEveryProperty(node, visitor);

	//std::cout << "Json Generated:\n" << std::setw(4) << visitor.result << std::endl;
}
