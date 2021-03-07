#pragma once
#include "core/iterable/IterableSafeVector.h"
#include "editor/CaptionMenuBar.h"
#include "editor/EdComponentWindows.h"
#include "editor/EditorCamera.h"
#include "editor/EdMenu.h"
#include "editor/EdOperation.h"
#include "editor/imgui/ImEd.h"
#include "editor/windows/EdWindow.h"

#include <nlohmann/json.hpp>
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
	void BeforePlayWorld(World& world);
	void AfterStopWorld(World& world);
	friend class Editor;
	friend struct ed::CaptionMenuBar;

	// PERF: use some faster representation.
	nlohmann::json m_lastPlayedWorld;

	void HandleInput();
	void Dockspace();

	void HandleClickSelection(bool wasCtrl);

	IterableSafeVector<std::function<void()>> m_postDrawCommands;
	std::vector<std::function<void()>> m_deferredCommands;

	BasePodHandle lastClickAssetSelected{};

} * EditorObject{};
