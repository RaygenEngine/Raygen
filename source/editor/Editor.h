#pragma once

#include "editor/SceneSave.h"
#include "editor/AssetWindow.h"

#include "editor/PropertyEditor.h"
#include "editor/NodeContextActions.h"
#include "system/EngineEvents.h"
#include "world/nodes/camera/EditorCameraNode.h"

#include <memory>
#include <functional>

class Node;
class PropertyEditor;
class AssetWindow;

class Editor {
public:
	struct ImMenu {
		ImMenu(const char* inName)
			: name(inName)
		{
		}

		const char* name;

		using FnPointer = std::function<void()>;

		struct MenuOption {
			const char* name{ nullptr };
			FnPointer func;
		};

		std::vector<MenuOption> options;

		void AddSeperator() { options.emplace_back(); }

		void AddEntry(const char* inName, FnPointer funcPtr)
		{
			ImMenu::MenuOption option;
			option.name = inName;
			option.func = funcPtr;
			options.emplace_back(option);
		}

		virtual void DrawOptions(Editor* editor)
		{
			for (auto& entry : options) {
				if (entry.name == nullptr) {
					ImGui::Separator();
					continue;
				}

				if (ImGui::MenuItem(entry.name)) {
					std::invoke(entry.func);
				}
			}
		}

		void Draw(Editor* editor)
		{
			if (ImGui::BeginMenu(name)) {
				ImGui::Spacing();
				DrawOptions(editor);
				ImGui::Spacing();
				ImGui::EndMenu();
			}
		}

		virtual ~ImMenu() = default;
	};

protected:
	bool m_updateWorld{ false };
	Node* m_selectedNode{ nullptr };

	bool m_showImguiDemo{ false };
	bool m_showGltfWindow{ false };

	bool m_showAboutWindow{ false };
	bool m_showHelpWindow{ false };
	bool m_showLogWindow{ false };


	bool m_autoRestoreWorld{ false };

	SceneSave m_sceneSave;

	std::unique_ptr<AssetWindow> m_assetWindow;
	std::unique_ptr<PropertyEditor> m_propertyEditor;

	ImGui::FileBrowser m_loadFileBrowser = ImGui::FileBrowser(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);

	void MakeMainMenu();

public:
	DECLARE_EVENT_LISTENER(m_onNodeRemoved, Event::OnWorldNodeRemoved);
	DECLARE_EVENT_LISTENER(m_onWorldLoaded, Event::OnWorldLoaded);

	EditorCameraNode* m_editorCamera;
	bool m_hasEditorCameraCachedMatrix{ false };
	glm::mat4 m_editorCameraCachedMatrix{ glm::identity<glm::mat4>() };

	std::unique_ptr<NodeContextActions> m_nodeContextActions;

	fs::path m_sceneToLoad{};

	std::vector<std::unique_ptr<ImMenu>> m_menus;

	Editor();
	virtual ~Editor();
	Editor(const Editor&) = delete;
	Editor(Editor&&) = delete;
	Editor& operator=(const Editor&) = delete;
	Editor& operator=(Editor&&) = delete;


	void UpdateEditor();

	// On Toggle
	void OnDisableEditor();
	void OnEnableEditor();

	[[nodiscard]] bool ShouldUpdateWorld() const { return m_updateWorld; }

	void PreBeginFrame();

	static void Duplicate(Node* node);
	static void Delete(Node* node);

	static void MoveChildUp(Node* node);
	static void MoveChildDown(Node* node);

	static void MoveChildOut(Node* node);
	static void MoveSelectedUnder(Node* node);
	static void MakeChildOf(Node* newParent, Node* node);

	static void PilotThis(Node* node);
	static void TeleportToCamera(Node* node);

	static void MakeActiveCamera(Node* node);

	void Run_ContextPopup(Node* node);
	void Run_NewNodeMenu(Node* underNode);
	void Run_AssetView();

	void Run_MaybeAssetTooltip(PodEntry* entry);
	void Run_OutlinerDropTarget(Node* node);

	void Run_MenuBar();

	void Run_AboutWindow();
	void Run_HelpWindow();

	void Run_LogWindow();
	static void PushCommand(std::function<void()>&& func);
	static void PushPostFrameCommand(std::function<void()>&& func);

	void OnPlay();
	void OnStopPlay();

private:
	void SpawnEditorCamera();
	void Outliner();

	void LoadScene(const fs::path& scenefile);
	void ReloadScene();

	void HandleInput();


	std::vector<std::function<void()>> m_postDrawCommands;
	std::vector<std::function<void()>> m_postFrameCommands;
};

template<typename Lambda>
void RecurseNodes(Node* root, Lambda f, int32 depth = 0)
{
	if (!root) {
		return;
	}

	f(root, depth);
	for (auto& c : root->GetChildren()) {
		RecurseNodes(c.get(), f, depth + 1);
	}
}
