#ifndef LIGHTNODE_H
#define LIGHTNODE_H

#include "world/nodes/Node.h"

namespace World
{
	class LightNode : public Node
	{
		glm::vec3 m_color;

	public:
		LightNode(Node* parent);
		~LightNode() = default;

		glm::vec3 GetColor() const { return m_color; }

		std::string ToString(bool verbose, uint depth) const override;

		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
	};
}

#endif // LIGHTNODE_H
