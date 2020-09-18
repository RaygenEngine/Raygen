#pragma once
#include "core/iterable/IterableSafeVector.h"
#include "editor/EdComponentWindows.h"
#include "editor/imgui/ImEd.h"
#include "editor/windows/EdWindow.h"

#include "editor/EdMenu.h"
#include "editor/EdOperation.h"
#include "editor/EditorCamera.h"


#include <memory>
#include <functional>


class EcsWorld;

inline class EditorObject_ : public Listener {
public:
protected:
	bool m_updateWorld{ false };

	bool m_autoRestoreWorld{ false };
	bool m_hasRestoreSave{ false };


	bool m_openPopupDeleteLocal{ false };

	void MakeMainMenu();

	ImGuiID m_dockspaceId;

	void UpdateViewportCoordsFromDockspace();
	bool m_isMaximised{ false };

public:
	bool m_hasEditorCameraCachedMatrix{ false };
	glm::mat4 m_editorCameraCachedMatrix{ glm::identity<glm::mat4>() };
	glm::mat4 m_editorCameraPrePilotPos{ glm::identity<glm::mat4>() };

	ed::Menu m_mainMenu{ "MainMenu" };

	EcsWorld* m_currentWorld{ nullptr };

	ed::ComponentWindows m_windowsComponent;

	EditorObject_();
	virtual ~EditorObject_();
	EditorObject_(const EditorObject_&) = delete;
	EditorObject_(EditorObject_&&) = delete;
	EditorObject_& operator=(const EditorObject_&) = delete;
	EditorObject_& operator=(EditorObject_&&) = delete;

	ed::EditorCamera edCamera;

	void UpdateEditor();

	// On Toggle
	void OnDisableEditor();
	void OnEnableEditor();

	[[nodiscard]] bool ShouldUpdateWorld() const { return m_updateWorld; }

	// static void SelectNode(Node* node);

	// static void MoveSelectedUnder(Node* node);

	// static void Duplicate(Node* node);
	// static void Delete(Node* node);

	// static void PilotThis(Node* node);
	// static void FocusNode(Node* node);
	// static void TeleportToCamera(Node* node);


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
		// WIP: ECS
		// if (m_editorCamera) {
		//	return !m_editorCamera->GetParent()->IsRoot();
		//}
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

	void TopMostMenuBarDraw();

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
