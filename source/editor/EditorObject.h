#pragma once
#include "core/iterable/IterableSafeVector.h"
#include "editor/EdComponentWindows.h"
#include "editor/imgui/ImEd.h"
#include "editor/windows/EdWindow.h"

#include "editor/EdMenu.h"
#include "editor/EdOperation.h"
#include "editor/EditorCamera.h"
#include "editor/CaptionMenuBar.h"


#include <memory>
#include <functional>


class World;

inline class EditorObject_ : public Listener {
public:
	EditorObject_();
	virtual ~EditorObject_();
	EditorObject_(const EditorObject_&) = delete;
	EditorObject_(EditorObject_&&) = delete;
	EditorObject_& operator=(const EditorObject_&) = delete;
	EditorObject_& operator=(EditorObject_&&) = delete;

protected:
	ImGuiID m_dockspaceId;

	void UpdateViewportCoordsFromDockspace();
	bool m_isMaximised{ false };

	bool m_drawUi{ true };

public:
	bool m_hasEditorCameraCachedMatrix{ false };
	glm::mat4 m_editorCameraCachedMatrix{ glm::identity<glm::mat4>() };
	glm::mat4 m_editorCameraPrePilotPos{ glm::identity<glm::mat4>() };

	World* m_currentWorld{ nullptr };

	ed::CaptionMenuBar m_captionBar;
	ed::ComponentWindows m_windowsComponent;


	ed::EditorCamera edCamera;

	void UpdateEditor();

	static void PushCommand(std::function<void()>&& func);
	static void PushDeferredCommand(std::function<void()>&& func);

	void SaveLevel();
	void SaveLevelAs();
	void SaveAll();
	void NewLevel();

	static bool EditorHandleKeyEvent(int32 glfwKey, int32 glfwScancode, int32 glfwAction, int32 glfwModifiers)
	{
		return false;
	}

	void OnFileDrop(std::vector<fs::path>&& files);

	void OpenLoadDialog();

private:
	friend struct ed::CaptionMenuBar;

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
