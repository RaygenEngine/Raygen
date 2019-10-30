#pragma once

#include "editor/Editor.h"
#include <imfilebrowser.h>
class Node;

class PropertyEditor {
public:
	struct FileOperation {
		ImGui::FileBrowser assetDialog;

		void* forObj{ nullptr };
		std::string forPropName;
		size_t forId{ 0 };

		fs::path filepath;

		void BeginDialogFor(void* reflObj, const Property& p, size_t id)
		{
			forObj = reflObj;
			forPropName = p.GetName();
			forId = id;
			filepath = "";

			assetDialog.Open();
		}

		bool HasFileFor(void* reflObj, const Property& p, size_t id)
		{
			return !filepath.empty() && reflObj == forObj && id == forId && forPropName == p.GetName();
		}

		void Display()
		{
			assetDialog.Display();
			if (assetDialog.HasSelected()) {
				filepath = assetDialog.GetSelected();
				assetDialog.ClearSelected();
			}
		}
	};

	bool m_localMode{ true };
	bool m_displayMatrix{ false };
	bool m_massEditMaterials{ false };
	bool m_lockedScale{ false };

	Node* m_prevNode{ nullptr };

	bool m_lookAtMode{ false };
	glm::vec3 m_lookAtPos{ 0.f, 0.f, 0.f };

	// Injects the imgui code of a property editor from a node.
	void Inject(Node* node);

	void Run_BaseProperties(Node* node);

	void Run_ContextActions(Node* node);

	void Run_ReflectedProperties(Node* node);

	FileOperation m_openAsset;
	FileOperation m_saveAsset;

	PropertyEditor()
	{
		m_saveAsset.assetDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_EnterNewFilename
													 | ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CreateNewDir
													 | ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);
	}
};
