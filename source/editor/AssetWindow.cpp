#include "pch.h"
#include "imgui/imgui.h"
#include "editor/AssetWindow.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "system/reflection/ReflectionTools.h"
#include "editor/imgui/ImguiExtensions.h"

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
			for (auto& s : m_gltf)
			{
				DrawFileAsset(n, s);
			}
		}
		if (ImGui::CollapsingHeader("Image files"))
		{
			for (auto& s : m_images)
			{
				DrawFileAsset(n, s);
			}
		}

		if (ImGui::CollapsingHeader("Scene files"))
		{
			for (auto& s : m_xscn)
			{
				DrawFileAsset(n, s);
			}
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



void AssetWindow::DrawFileAsset(int32& n, const std::string& path)
{
	ImGui::PushID(n++);
	ImGui::Button(path.c_str());

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("ASSET_PATH", &path, sizeof(std::string));
		ImGui::EndDragDropSource();
	}
	ImGui::PopID();
}

#include "asset/PodIncludes.h"

namespace
{


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

		if (open)
		{
			GenerateUniqueName(p);
			ImGui::InputText(name, &str, ImGuiInputTextFlags_ReadOnly);


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
				if (ImGui::CollapsingHeader(finalName.c_str()))
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


template<typename T>
void Visit(T* pod, int32& id)
{
	ImGui::PushID(id++);
	if (ImGui::CollapsingHeader(GetReflector(pod).GetName().c_str()))
	{
		ImGui::Indent();
		CallVisitorOnEveryProperty(pod, ReflectionToImguiVisitor());
		ImGui::Unindent();
		
	}
	ImGui::PopID();
}

void CallVisit(AssetPod* pod, int32& id)
{
#define MAYBE_VISIT(Type) if (typeid(*pod) == typeid(Type)) { Visit<Type>(dynamic_cast<Type*>(pod), id); }

	MAYBE_VISIT(ImagePod);
	MAYBE_VISIT(MaterialPod);
	MAYBE_VISIT(ModelPod);
	MAYBE_VISIT(ShaderPod);
	MAYBE_VISIT(StringPod);
	MAYBE_VISIT(TexturePod);

#undef MAYBE_VISIT
}
};

void DrawPodList(size_t hash, std::string& name, std::vector<AssetPod*>& pods)
{
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		int32 id = 0;
		for (auto pod : pods)
		{
			CallVisit(pod, id);
		}
	}
}

void AssetWindow::DrawAssetLibrary()
{
	if (ImGui::CollapsingHeader("Assets"))
	{
		m_podLists.clear();
		for (auto& [uid, pod] : Engine::GetAssetManager()->m_uidToPod)
		{
			DetectPodCategory(pod);
		}
		
		for (auto& [hash, list] : m_podLists)
		{
			DrawPodList(hash, m_knownPodTypes[hash], list);
		}
	}

}

void AssetWindow::DrawAssetPod(AssetPod* pod)
{

}

void AssetWindow::DetectPodCategory(AssetPod* pod)
{
	const std::type_info& ti = typeid(*pod);
	size_t hash = ti.hash_code();

	auto it = m_knownPodTypes.find(hash);
	
	if (it == m_knownPodTypes.end())
	{
		it = m_knownPodTypes.insert({ hash, ti.name() }).first;
		m_podLists.insert({ hash, {} });
	}

	m_podLists[hash].push_back(pod);
}
