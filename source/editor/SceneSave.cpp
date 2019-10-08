#include "pch.h"
#include "editor/SceneSave.h"
#include "imgui/imgui.h"
#include "system/Engine.h"
#include "tinyxml2/tinyxml2.h"
#include "world/nodes/Node.h"
#include "world/World.h"
#include "core/reflection/ReflectionTools.h"
#include "asset/AssetManager.h"
#include "editor/Editor.h"
#include "asset/PodIncludes.h"
#include "asset/UriLibrary.h"
#include "GLM/glm.hpp"

#include <string>

SceneSave::SceneSave()
{
	m_saveBrowser = ImGui::FileBrowser(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_EnterNewFilename
									   | ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CreateNewDir
									   | ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);
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
	if (m_saveBrowser.HasSelected()) {
		fs::path file = m_saveBrowser.GetSelected();
		m_saveBrowser.ClearSelected();
		SaveAsXML(Engine::GetWorld(), file.string());
	}
}

namespace {

struct GenerateXMLVisitor {
	tinyxml2::XMLElement* xmlElement;
	const char* name;
	std::string name_str; // required for const char* validility

	bool PreProperty(const Property& p)
	{
		if (p.HasFlags(PropertyFlags::NoSave)) {
			return false;
		}
		name_str = p.GetNameStr();
		name = name_str.c_str();
		return true;
	}

	void operator()(int32& v, const Property& p) { xmlElement->SetAttribute(name, v); }

	void operator()(bool& v, const Property& p) { xmlElement->SetAttribute(name, v); }

	void operator()(float& v, const Property& p) { xmlElement->SetAttribute(name, v); }

	void operator()(glm::vec3& v, const Property& p)
	{
		xmlElement->SetAttribute(name, ParsingAux::FloatsToString(v).c_str());
	}

	void operator()(glm::vec4& v, const Property& p)
	{
		xmlElement->SetAttribute(name, ParsingAux::FloatsToString(v).c_str());
	}

	void operator()(std::string& v, const Property& p) { xmlElement->SetAttribute(name, v.c_str()); }

	template<typename T>
	void operator()(PodHandle<T>& handle, const Property& p)
	{
		auto podPath = AssetManager::GetPodUri(handle);
		xmlElement->SetAttribute(name, podPath.c_str());
	}

	template<typename T>
	void operator()(T& v, const Property& p)
	{
		LOG_WARN("Attempting to save an unimplemented save type, skipping property: ", p.GetName());
	}
};


tinyxml2::XMLElement* GenerateNodeXML(Node* node, tinyxml2::XMLDocument& document)
{
	tinyxml2::XMLElement* xmlElem;

	xmlElem = document.NewElement(node->GetType().c_str());
	xmlElem->SetAttribute("name", node->GetName().c_str());

	xmlElem->SetAttribute("translation", ParsingAux::FloatsToString(node->GetLocalTranslation()).c_str());
	xmlElem->SetAttribute("euler_pyr", ParsingAux::FloatsToString(node->GetLocalPYR()).c_str());
	xmlElem->SetAttribute("scale", ParsingAux::FloatsToString(node->GetLocalScale()).c_str());

	GenerateXMLVisitor visitor;
	visitor.xmlElement = xmlElem;

	refltools::CallVisitorOnEveryProperty(node, visitor);
	
	return xmlElem;
}
} // namespace

bool SceneSave::SaveAsXML(World* world, const uri::Uri& path)
{
	using namespace tinyxml2;
	XMLDocument xmlDoc;

	std::unordered_map<Node*, XMLElement*> nodeXmlElements;

	auto root = Engine::GetWorld()->GetRoot();
	auto rootXml = GenerateNodeXML(root, xmlDoc);
	xmlDoc.InsertFirstChild(rootXml);

	nodeXmlElements.insert({ root, rootXml });


	RecurseNodes(root, [&](Node* node, auto) {
		if (node == root) {
			return;
		}

		auto xmlElem = GenerateNodeXML(node, xmlDoc);
		nodeXmlElements.insert({ node, xmlElem });

		Node* parent = node->GetParent();
		nodeXmlElements[parent]->InsertEndChild(xmlElem);
	});

	FILE* fp;
	if (fopen_s(&fp, path.c_str(), "w") == 0) {
		xmlDoc.SaveFile(fp);
		fclose(fp);
	}
	else {
		LOG_ERROR("Failed to open file for saving scene.");
	}


	return false;
}
