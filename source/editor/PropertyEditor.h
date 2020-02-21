#pragma once

#include "editor/Editor.h"
#include "reflection/Property.h"
class Node;

class PropertyEditor {
public:
	bool m_localMode{ true };
	bool m_displayMatrix{ false };
	bool m_massEditMaterials{ false };
	bool m_lockedScale{ false };

	Node* m_prevNode{ nullptr };

	bool m_lookAtMode{ false };
	glm::vec3 m_lookAtPos{ 0.f, 0.f, 0.f };

	// Injects the imgui code of a property editor from a node.
	void Inject(Node* node);

	void Run_BaseProperties(Node* node);

	void Run_ContextActions(Node* node);

	void Run_ReflectedProperties(Node* node);
};
