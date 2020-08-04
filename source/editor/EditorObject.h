#pragma once
#include "core/iterable/IterableSafeVector.h"
#include "editor/EdComponentWindows.h"
#include "editor/imgui/ImEd.h"
#include "editor/NodeContextActions.h"
#include "editor/windows/EdWindow.h"
#include "universe/nodes/camera/EditorCameraNode.h"
#include "editor/EdMenu.h"
#include "editor/EdOperation.h"

#include <memory>
#include <functional>

class Node;
class AssetWindow;

inline class EditorObject_ : public Listener {
public:
	static Node* GetSelectedNode();

public:
protected:
	bool m_updateWorld{ false };
	Node* m_selectedNode{ nullptr };

	bool m_autoRestoreWorld{ false };
	bool m_hasRestoreSave{ false };

	void MakeMainMenu();

	ImGuiID m_dockspaceId;

	void UpdateViewportCoordsFromDockspace();

public:
	EditorCameraNode* m_editorCamera;
	bool m_hasEditorCameraCachedMatrix{ false };
	glm::mat4 m_editorCameraCachedMatrix{ glm::identity<glm::mat4>() };
	glm::mat4 m_editorCameraPrePilotPos{ glm::identity<glm::mat4>() };

	UniquePtr<NodeContextActions> m_nodeContextActions;

	ed::Menu m_mainMenu{ "MainMenu" };

	ECS_World* m_currentWorld{ nullptr };

	ed::ComponentWindows m_windowsComponent;

	EditorObject_();
	virtual ~EditorObject_();
	EditorObject_(const EditorObject_&) = delete;
	EditorObject_(EditorObject_&&) = delete;
	EditorObject_& operator=(const EditorObject_&) = delete;
	EditorObject_& operator=(EditorObject_&&) = delete;


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

	void SaveLevel();
	void SaveLevelAs();
	void SaveAll();
	void NewLevel();

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

} * EditorObject{};

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
