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

		auto str = Engine::GetAssetManager()->GetPodPath(pod).string();
		bool open = ImGui::CollapsingHeader(name);

		if (open)
		{
			GenerateUniqueName(p);
			ImGui::InputText(name, &str, ImGuiInputTextFlags_ReadOnly);


			depth++;
			ImGui::Indent();

			refltools::CallVisitorOnEveryProperty(pod.operator->(), *this);

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
				std::string finalName = Engine::GetAssetManager()->GetPodPath(handle).string() + "##" + name;
				if (ImGui::CollapsingHeader(finalName.c_str()))
				{
					ImGui::Indent();
					refltools::CallVisitorOnEveryProperty(handle.operator->(), *this);
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

namespace {
void PodDragSource(AssetPod* pod, size_t uid)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		std::string payloadTag = "POD_UID_" + std::to_string(pod->type.hash());
		ImGui::SetDragDropPayload(payloadTag.c_str(), &uid, sizeof(size_t));
		ImGui::EndDragDropSource();
	}
}
}

void AssetWindow::DrawAssetLibrary()
{
	if (ImGui::CollapsingHeader("Assets"))
	{
		std::unordered_map<ctti::type_id_t, std::vector<std::pair<size_t, AssetPod*>>> podVectors;
		podtools::ForEachPodType([&](auto dummy) {
			using PodType = std::remove_pointer_t<decltype(dummy)>;

			podVectors.insert({ ctti::type_id<PodType>(), {} });
		});
		// TODO:
		//for (auto& [uid, pod] : Engine::GetAssetManager()->m_pods)
		//{
		//	podVectors[pod->type].push_back({ uid, pod });
		//}
		//int32 outerId = 0;
		//for (auto& [type, vector] : podVectors)
		//{
		//	outerId++;
		//	std::string name = type.name().cppstring();

		//	if (ImGui::CollapsingHeader(name.c_str()))
		//	{
		//		int32 n = 0 + outerId * 10000;
		//		ImGui::Indent();
		//		for (auto& p : vector)
		//		{

		//			auto& uid = p.first;
		//			auto& pod = p.second;

		//			
		//			std::string podPath = Engine::GetAssetManager()->GetPodPathFromId(uid);
		//			ImGui::PushID(n++);

		//			bool open = ImGui::CollapsingHeader(podPath.c_str());
		//			PodDragSource(pod, uid);
		//			if (open)
		//			{
		//				ImGui::Indent();
		//				refltools::CallVisitorOnEveryProperty(pod, ReflectionToImguiVisitor());
		//				ImGui::Unindent();
		//			}
		//			ImGui::PopID();


		//		}
		//		ImGui::Unindent();
		//	}
		//}
	}

	
}
