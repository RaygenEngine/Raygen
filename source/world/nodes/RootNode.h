#pragma once
#include "world/nodes/Node.h"

class RootNode : public Node {

	REFLECTED_NODE(RootNode, Node)
	{
		REFLECT_VAR(m_background, PropertyFlags::Color);
		REFLECT_VAR(m_ambient, PropertyFlags::Color);
	}

	glm::vec3 m_background{ 0.2f, 0.2f, 0.4f };
	glm::vec3 m_ambient{ 0.2f, 0.2f, 0.4f };

public:
	RootNode()
		: Node(nullptr)
	{
	}

	[[nodiscard]] glm::vec3 GetBackgroundColor() const { return m_background; }
	[[nodiscard]] glm::vec3 GetAmbientColor() const { return m_ambient; }

	void SetBackgroundColor(const glm::vec3& color) { m_background = color; }
	void SetAmbientColor(const glm::vec3& color) { m_ambient = color; }

	~RootNode() override { m_children.clear(); }
};
