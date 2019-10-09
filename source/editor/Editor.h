#pragma once

#include "platform/Window.h"
#include "editor/SceneSave.h"
#include "editor/AssetWindow.h"

#include <filesystem>
namespace fs = std::filesystem;
class Node;

class Editor {
protected:
	bool m_updateWorld{ true };
	Node* m_selectedNode{ nullptr };

	SceneSave m_sceneSave;

	AssetWindow m_assetWindow;

public:
	bool m_showImgui{ true };
	fs::path m_sceneToLoad{};

	Editor();
	virtual ~Editor();
	Editor(const Editor&) = default;
	Editor(Editor&&) = default;
	Editor& operator=(const Editor&) = default;
	Editor& operator=(Editor&&) = default;


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
	for (auto c : root->GetChildren()) {
		RecurseNodes(c.get(), f, depth + 1);
	}
}
