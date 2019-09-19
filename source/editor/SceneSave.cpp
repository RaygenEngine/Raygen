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


namespace
{

struct GenerateXMLVisitor
{
	tinyxml2::XMLElement* xmlElement;
	const char* name;

	void Visit(int32& v, ExactProperty& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), v);
	}

	void Visit(bool& v, ExactProperty& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), v);
	}

	void Visit(float& v, ExactProperty& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), v);
	}

	void Visit(glm::vec3& v, ExactProperty& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), ParsingAux::FloatsToString(v).c_str());
	}

	void Visit(glm::vec4& v, ExactProperty& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), ParsingAux::FloatsToString(v).c_str());
	}	
	
	void Visit(std::string& v, ExactProperty& p)
	{
		xmlElement->SetAttribute(p.GetName().c_str(), v.c_str());
	}

	template<typename T>
	void Visit(PodHandle<T>& handle, ExactProperty& p)
	{
		auto podPath = Engine::GetAssetManager()->GetPodPath(handle);
		xmlElement->SetAttribute(p.GetName().c_str(), uri::RemovePathDirectory(podPath).string().c_str());
	}

	template<typename T>
	void Visit(T& v, ExactProperty& p)
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

	for (auto& p : GetReflector(node).GetProperties())
	{
		if (!p.HasFlags(PropertyFlags::NoSave))
		{
			CallVisitorOnProperty(p, visitor);
		}
	}

	return xmlElem;
}
}

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
