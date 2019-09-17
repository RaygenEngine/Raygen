#pragma once

#include "platform/Window.h"

class Node;

class Editor
{
protected:
	bool m_updateWorld;
	Node* m_selectedNode;
	
public:

	bool m_showImgui{ true };
	std::string m_sceneToLoad{};

	Editor();

	virtual ~Editor();

	void UpdateEditor();

	bool ShouldUpdateWorld() const 
	{
		return m_updateWorld;
	}

	void SaveScene(const std::string& filename);

	void PreBeginFrame();

private:
	void Outliner();
	void PropertyEditor(Node* activeNode);
	void LoadScene(const std::string& scenefile);
};
