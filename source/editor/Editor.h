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
	bool m_updateWorld{ true };
	Node* m_selectedNode{ nullptr };

	SceneSave m_sceneSave;

	std::unique_ptr<AssetWindow> m_assetWindow;
	std::unique_ptr<PropertyEditor> m_propertyEditor;

public:
	DECLARE_EVENT_LISTENER(m_onNodeRemoved, Event::OnWorldNodeRemoved);

	std::unique_ptr<NodeContextActions> m_nodeContextActions;

	fs::path m_sceneToLoad{};

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
