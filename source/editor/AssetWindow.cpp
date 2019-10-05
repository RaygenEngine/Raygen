#include "pch.h"
#include "imgui/imgui.h"
#include "editor/AssetWindow.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "core/reflection/ReflectionTools.h"
#include "editor/imgui/ImguiExtensions.h"
#include "core/reflection/PodTools.h"


void AssetWindow::Init()
{
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".gltf", m_gltf);
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".xscn", m_xscn);

	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".jpg", m_images);
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".png", m_images);
	Engine::GetAssetManager()->m_pathSystem.GenerateFileListOfType(".tga", m_images);
}

void AssetWindow::DrawFileLibrary()
{
	int32 n = 0;

	if (ImGui::CollapsingHeader("Files"))
	{
		if (ImGui::CollapsingHeader("Model files"))
		{
			ImGui::Indent();
			for (auto& s : m_gltf)
			{
				DrawFileAsset(n, s);
			}
			ImGui::Unindent();
		}
		if (ImGui::CollapsingHeader("Image files"))
		{
			ImGui::Indent();
			for (auto& s : m_images)
			{
				DrawFileAsset(n, s);
			}
			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Scene files"))
		{
			ImGui::Indent();
			for (auto& s : m_xscn)
			{
				DrawFileAsset(n, s);
			}
			ImGui::Unindent();
		}
	}
}

void AssetWindow::Draw()
{
	ImGui::Begin("Asset Window");
	DrawFileLibrary();
	DrawAssetLibrary();
	ImGui::End();

	for (auto& open : m_openFiles)
	{
		DrawEditor(open);
	}

	for (auto& toClose : m_openFilesRemove)
	{
		m_openFiles.erase(toClose);
	}
	m_openFilesRemove.clear();
}

#include "asset/UriLibrary.h"
void AssetWindow::DrawFileAsset(int32& n, const std::string& path)
{
	ImGui::PushID(n++);

	if (uri::MatchesExtension(path, ".gltf"))
	{
		ImGui::Button(path.c_str());
			

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			auto h = AssetManager::GetOrCreate<ModelPod>(path + "/#model");

			std::string payloadTag = "POD_UID_" + std::to_string(h->type.hash());
			ImGui::SetDragDropPayload(payloadTag.c_str(), &h.podId, sizeof(size_t));
			ImGui::EndDragDropSource();
		}
	}
	else
	{
		ImGui::Button(path.c_str());
	}

	ImGui::PopID();
}

#include "core/reflection/PodReflection.h"


namespace
{
using namespace PropertyFlags;

struct ReflectionToImguiVisitor
{
	int32 depth{ 0 };
	std::string path{};

	std::set<std::string> objNames;

	std::string nameBuf;
	const char* name;


	bool dirty{ false };
	bool recurse{ true };
	ReflectionToImguiVisitor(bool recursive = true)
		: recurse(recursive)
	{}

	void GenerateUniqueName(const Property& p)
	{
		std::string buf = p.GetNameStr() + "##" + path;
		auto r = objNames.insert(buf);
		int32 index = 0;
		while (r.second == false)
		{
			r = objNames.insert(buf + std::to_string(index));
			index++;
		}
		name = r.first->c_str();
	}

	void Begin(const ReflClass& r)
	{
		path += "|" + r.GetNameStr();
	}

	void End(const ReflClass& r)
	{
		path.erase(path.end() - (r.GetNameStr().size() + 1), path.end());
	}

	void PreProperty(const Property& p)
	{
		GenerateUniqueName(p);
	}

	template<typename T>
	void operator()(T& t, const Property& p)
	{
		if (Inner(t, p))
		{
			dirty = true;
		}
	}

	bool Inner(int32& t, const Property& p)
	{
		return ImGui::DragInt(name, &t, 0.1f);
	}

	bool Inner(bool& t, const Property& p)
	{
		if (p.HasFlags(NoEdit))
		{
			bool t1 = t;
			ImGui::Checkbox(name, &t1);
			return false;
		}
		return ImGui::Checkbox(name, &t);
	}

	bool Inner(float& t, const Property& p)
	{
		return ImGui::DragFloat(name, &t, 0.01f);
	}

	bool Inner(glm::vec3& t, const Property& p)
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

	bool Inner(glm::vec4& t, const Property& p)
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

	bool Inner(std::string& ref, const Property& p)
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
	bool Inner(PodHandle<PodType>& pod, const Property& p)
	{
		if (!pod.HasBeenAssigned())
		{
			std::string s = "Unitialised handle: " + p.GetNameStr();
			ImGui::Text(s.c_str());
			return false;
		}

		auto str = AssetManager::GetPodUri(pod);

		bool open = ImGui::CollapsingHeader(name);
		if (open)
		{
			GenerateUniqueName(p);
			ImGui::InputText(name, &str, ImGuiInputTextFlags_ReadOnly);


			depth++;
			ImGui::Indent();
			if (recurse)
			{
				refltools::CallVisitorOnEveryProperty(pod.operator->(), *this);
			}
			ImGui::Unindent();
			depth--;
		}
		return false;
	}

	template<typename T>
	bool Inner(T& t, const Property& p)
	{
		std::string s = "unhandled property: " + p.GetNameStr();
		ImGui::Text(s.c_str());
		return false;
	}

	template<typename T>
	bool Inner(std::vector<PodHandle<T>>& t, const Property& p)
	{
		if (ImGui::CollapsingHeader(name))
		{
			ImGui::Indent();
			int32 index = 0;
			for (auto& handle : t)
			{
				++index;
				std::string sname = "|" + p.GetNameStr() + std::to_string(index);
				size_t len = sname.size();
				path += sname;

				GenerateUniqueName(p);
				std::string finalName = AssetManager::GetPodUri(handle) + "##" + name;
				if (ImGui::CollapsingHeader(finalName.c_str()))
				{
					ImGui::Indent();
					if (recurse)
					{
						refltools::CallVisitorOnEveryProperty(handle.operator->(), *this);
					}
					ImGui::Unindent();
				}

				path.erase(path.end() - (len), path.end());
			}
			ImGui::Unindent();
		}
		return false;
	}


	// Enum
	bool Inner(MetaEnumInst& t, const Property& p)
	{
		auto enumMeta = t.GetEnum();

		//int32 currentItem = ;
		std::string str;
		str = t.GetValueStr();
		int32 currentValue = t.GetValue();

		if (ImGui::BeginCombo(name, str.c_str())) // The second parameter is the label previewed before opening the combo.
		{
			for (auto& [enumStr, value] : enumMeta.GetStringsToValues())
			{
				bool selected = (currentValue == value);
				if (ImGui::Selectable(enumStr.c_str(), &selected))
				{
					t.SetValue(value);
				}
				if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return false;
	}
};

}

namespace {
void PodDragSource(PodEntry* entry)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		std::string payloadTag = "POD_UID_" + std::to_string(entry->type.hash());
		ImGui::SetDragDropPayload(payloadTag.c_str(), &entry->uid, sizeof(size_t));
		ImGui::EndDragDropSource();
	}
}
}

void AssetWindow::DrawAssetLibrary()
{
	if (ImGui::CollapsingHeader("Assets"))
	{

		ImGui::Checkbox("Recursive Pods", &recurse);

		ImGui::Indent();
		std::unordered_map<ctti::type_id_t, std::vector<PodEntry*>> podVectors;
		podtools::ForEachPodType([&](auto dummy) {
			using PodType = std::remove_pointer_t<decltype(dummy)>;

			podVectors.insert({ ctti::type_id<PodType>(), {} });
		});

		for (auto& pod : Engine::GetAssetManager()->m_pods)
		{
			podVectors[pod->type].push_back({ pod.get() });
		}
		int32 outerId = 0;
		for (auto& [type, vector] : podVectors)
		{
			outerId++;
			std::string name = type.name().cppstring();

			if (ImGui::CollapsingHeader(name.c_str()))
			{
				int32 n = 0 + outerId * 10000;
				ImGui::Indent();
				for (auto& entry : vector)
				{
					ImGui::PushID(n++);

					bool open = ImGui::CollapsingHeader(entry->path.c_str());
					PodDragSource(entry);
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted("Right click for context menu.\nDrag and drop in node properties to replace Node handle references.");
						ImGui::PopTextWrapPos();
						ImGui::EndTooltip();
					}
					if (ImGui::BeginPopupContextItem("Asset Context"))
					{
						if (ImGui::Selectable("Edit"))
						{
							m_openFiles.emplace(entry);
						}
						if (ImGui::Selectable("Clone"))
						{
							ImGui::OpenPopup("Clone Name");
						}

						if (ImGui::BeginPopupModal("Clone Name", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							ImGui::Text("Give a name:");
							ImGui::Separator();
							std::string outString;
							ImGui::InputText("Name", &outString);

							if (ImGui::Button("OK", ImVec2(120, 0))) 
							{ 
								LOG_FATAL("{}", outString);
								ImGui::CloseCurrentPopup(); 
							}
							
							ImGui::SetItemDefaultFocus();
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) 
							{ 
								ImGui::CloseCurrentPopup(); 
							}
							ImGui::EndPopup();
						}

						ImGui::EndPopup();
					}

					if (open)
					{
						ImGui::Indent();
						refltools::CallVisitorOnEveryProperty(entry->ptr.get(), ReflectionToImguiVisitor(recurse));
						ImGui::Unindent();
					}
					ImGui::PopID();
				}
				ImGui::Unindent();
			}
		}
		ImGui::Unindent();
	}

	
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
	j = AssetManager::GetPodUri(handle);
}

template<typename T>
void from_json(const json& j, PodHandle<T>& handle)
{
	handle = AssetManager::GetOrCreate(j.get<std::string>());
}

struct SerializeJsonVisitor
{
	SerializeJsonVisitor(json& r)
		: result(r) {}

	json& result;

	bool PreProperty(const Property& p)
	{
		if (p.HasFlags(PropertyFlags::NoSave))
		{
			return false;
		}
		return true;
	}

	template<typename T>
	void operator()(T& value, const Property& prop)
	{
		result[prop.GetNameStr()] = value;
	}

	template<typename T>
	void operator()(PodHandle<T>& value, const Property& prop)
	{
		result[prop.GetNameStr()] = value;
	}

	template<typename T>
	void operator()(std::vector<PodHandle<T>>& value, const Property& prop)
	{
		result[prop.GetNameStr()] = value;
	}

	void operator()(MetaEnumInst& inst, const Property& prop)
	{
		result[prop.GetNameStr()] = inst.GetValueStr();
	}
};
#include <iostream>
namespace
{
	void SaveAsJson(PodEntry* entry)
	{
		json result;
		podtools::VisitPod(entry->ptr.get(),
						   [&](auto p)
		{
			result["+type"] = entry->type.name().str();
		});
		result["+base"] = entry->path;

		refltools::CallVisitorOnEveryProperty(entry->ptr.get(), SerializeJsonVisitor(result));

		std::cout << "Json Generated:\n" << std::setw(4) << result << std::endl;
	}

	void LoadFromJson(PodEntry* entry)
	{
		LOG_ERROR("Load from json");
	}
}

void AssetWindow::DrawEditor(PodEntry* entry)
{
	std::string n = "Generic Editor: " + entry->path;
	
	bool open = true;
	ImGui::Begin(n.c_str(), &open);
	if (!open)
	{
		m_openFilesRemove.emplace(entry);
		ImGui::End();
		return;
	}
	auto am = Engine::GetAssetManager();
	if (entry->ptr)
	{
		if (ImGui::Button("Save Json"))
		{
			SaveAsJson(entry);
		}
		ImGui::SameLine();
		if (ImGui::Button("Load from Json"))
		{
			LoadFromJson(entry);
		}

		refltools::CallVisitorOnEveryProperty(entry->ptr.get(), ReflectionToImguiVisitor(false));
	}
	else
	{
		ImGui::Text("This asset is not loaded.");
	}
	ImGui::End();
}