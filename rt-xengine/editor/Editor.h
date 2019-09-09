#pragma once

#include "platform/Window.h"

class Node;

class Editor
{
protected:
	bool m_updateWorld;
	Node* m_selectedNode;
public:
	Editor();

	virtual ~Editor() = default;

	void UpdateEditor();

	bool ShouldUpdateWorld() const 
	{
		return m_updateWorld;
	}

	void SaveScene(const std::string& filename);

private:
	void Outliner();
	void PropertyEditor(Node* activeNode);
		
};
