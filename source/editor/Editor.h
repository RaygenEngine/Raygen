#pragma once

#include "editor/SceneSave.h"
#include "editor/AssetWindow.h"

#include "editor/PropertyEditor.h"
#include "editor/NodeContextActions.h"
#include "system/EngineEvents.h"

#include <memory>
#include <functional>

class Node;
class PropertyEditor;
class AssetWindow;

class Editor {
protected:
	struct ImMenu {
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

		void Draw()
		{
			if (ImGui::BeginMenu(name)) {
				ImGui::Spacing();
				for (auto& entry : options) {
					if (entry.name == nullptr) {
						ImGui::Separator();
						continue;
					}

					if (ImGui::MenuItem(entry.name)) {
						std::invoke(entry.func);
					}
				}
				ImGui::Spacing();
				ImGui::EndMenu();
			}
		}
	};

	bool m_updateWorld{ true };
	Node* m_selectedNode{ nullptr };

	bool m_showImguiDemo{ false };
	bool m_showGltfWindow{ false };

	bool m_showAboutWindow{ false };
	bool m_showHelpWindow{ false };

	SceneSave m_sceneSave;

	std::unique_ptr<AssetWindow> m_assetWindow;
	std::unique_ptr<PropertyEditor> m_propertyEditor;

	ImGui::FileBrowser m_loadFileBrowser = ImGui::FileBrowser(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);

	void MakeMainMenu();

public:
	DECLARE_EVENT_LISTENER(m_onNodeRemoved, Event::OnWorldNodeRemoved);

	std::unique_ptr<NodeContextActions> m_nodeContextActions;

	fs::path m_sceneToLoad{};

	std::vector<ImMenu> m_menus;

	Editor();
	virtual ~Editor();
	Editor(const Editor&) = delete;
	Editor(Editor&&) = delete;
	Editor& operator=(const Editor&) = delete;
	Editor& operator=(Editor&&) = delete;


	void UpdateEditor();

	[[nodiscard]] bool ShouldUpdateWorld() const { return m_updateWorld; }

	void PreBeginFrame();

	static void Duplicate(Node* node);
	static void Delete(Node* node);

	static void MoveChildUp(Node* node);
	static void MoveChildDown(Node* node);

	static void MoveChildOut(Node* node);
	static void MoveSelectedUnder(Node* node);
	static void MakeChildOf(Node* newParent, Node* node);

	static void LookThroughThis(Node* node);
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

private:
	void Outliner();
	void LoadScene(const fs::path& scenefile);

	void HandleInput();

	static void PushCommand(std::function<void()>&& func);


	std::vector<std::function<void()>> m_postDrawCommands;
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
