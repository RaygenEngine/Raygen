#pragma once

#include "editor/SceneSave.h"
#include "editor/AssetWindow.h"

class Node;

class Editor {
protected:
	bool m_updateWorld{ true };
	Node* m_selectedNode{ nullptr };

	SceneSave m_sceneSave;

	AssetWindow m_assetWindow;

public:
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

private:
	void Outliner();
	void PropertyEditor(Node* activeNode);
	void LoadScene(const fs::path& scenefile);

	void HandleInput();
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
