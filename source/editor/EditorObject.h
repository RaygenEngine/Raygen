#pragma once
#include "core/iterable/IterableSafeVector.h"
#include "editor/EdComponentWindows.h"
#include "editor/imgui/ImEd.h"
#include "editor/NodeContextActions.h"
#include "editor/SceneSave.h"
#include "editor/windows/EdWindow.h"
#include "universe/nodes/camera/EditorCameraNode.h"

#include <memory>
#include <functional>

class Node;
class AssetWindow;

inline class EditorObject : public Listener {
public:
	static Node* GetSelectedNode();

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
			std::function<bool()> isSelectedFunc;

			MenuOption()
			{
				isSelectedFunc = []() {
					return false;
				};
			}
		};

		std::vector<MenuOption> options;

		void AddSeperator() { options.emplace_back(); }

		void AddEntry(const char* inName, FnPointer funcPtr, std::function<bool()> isSelectedBind = {})
		{
			ImMenu::MenuOption option;
			option.name = inName;
			option.func = funcPtr;
			if (isSelectedBind) {
				option.isSelectedFunc = isSelectedBind;
			}

			options.emplace_back(option);
		}

		virtual void DrawOptions(EditorObject* editor)
		{
			for (auto& entry : options) {
				if (entry.name == nullptr) {
					ImGui::Separator();
					continue;
				}

				if (ImGui::MenuItem(entry.name, nullptr, entry.isSelectedFunc())) {
					std::invoke(entry.func);
				}
			}
		}

		void Draw(EditorObject* editor)
		{
			bool open = ImEd::BeginMenu(name);
			if (open) {
				DrawOptions(editor);
				ImEd::EndMenu();
			}
		}

		virtual ~ImMenu() = default;
	};

protected:
	bool m_updateWorld{ false };
	Node* m_selectedNode{ nullptr };

	bool m_autoRestoreWorld{ false };
	bool m_hasRestoreSave{ false };

	SceneSave m_sceneSave;

	void MakeMainMenu();

	ImGuiID m_dockspaceId;

	void UpdateViewportCoordsFromDockspace();

public:
	EditorCameraNode* m_editorCamera;
	bool m_hasEditorCameraCachedMatrix{ false };
	glm::mat4 m_editorCameraCachedMatrix{ glm::identity<glm::mat4>() };
	glm::mat4 m_editorCameraPrePilotPos{ glm::identity<glm::mat4>() };

	UniquePtr<NodeContextActions> m_nodeContextActions;

	std::vector<UniquePtr<ImMenu>> m_menus;

	ed::ComponentWindows m_windowsComponent;

	EditorObject();
	virtual ~EditorObject();
	EditorObject(const EditorObject&) = delete;
	EditorObject(EditorObject&&) = delete;
	EditorObject& operator=(const EditorObject&) = delete;
	EditorObject& operator=(EditorObject&&) = delete;


	void UpdateEditor();

	// On Toggle
	void OnDisableEditor();
	void OnEnableEditor();

	[[nodiscard]] bool ShouldUpdateWorld() const { return m_updateWorld; }

	static void SelectNode(Node* node);

	static void MoveSelectedUnder(Node* node);

	static void Duplicate(Node* node);
	static void Delete(Node* node);

	static void PilotThis(Node* node);
	static void FocusNode(Node* node);
	static void TeleportToCamera(Node* node);


	void Run_MenuBar();

	static void PushCommand(std::function<void()>&& func);
	static void PushDeferredCommand(std::function<void()>&& func);

	void OnPlay();
	void OnStopPlay();

	[[nodiscard]] bool IsCameraPiloting() const
	{
		if (m_editorCamera) {
			return !m_editorCamera->GetParent()->IsRoot();
		}
		return false;
	}

	static bool EditorHandleKeyEvent(int32 glfwKey, int32 glfwScancode, int32 glfwAction, int32 glfwModifiers)
	{
		return false;
	}

	void OnFileDrop(std::vector<fs::path>&& files);


private:
	void SpawnEditorCamera();

	void OpenLoadDialog();
	void ReloadScene();

	void HandleInput();
	void Dockspace();

	IterableSafeVector<std::function<void()>> m_postDrawCommands;
	std::vector<std::function<void()>> m_deferredCommands;

} * EditorObj;

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
