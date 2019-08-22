#ifndef OCULUSUSERNODE_H
#define OCULUSUSERNODE_H

#include "OculusHeadNode.h"
#include "world/nodes/Node.h"
#include "world/nodes/user/UserNode.h"

namespace World
{
	enum OCULUS_NAVIGATION_MODE : uint32
	{
		// default as it is easier to navigate, with basic equipment
		ONM_BODY_MOVEMENT_BASED_ON_HEAD_ORIENTATION= 0,
		ONM_BODY_MOVEMENT_BASED_ON_BODY_ORIENTATION,
		ONM_COUNT
	};

	class OculusUserNode : public UserNode
	{
		OCULUS_NAVIGATION_MODE m_navigationMode;

		OculusHeadNode* m_head;

	public:
		OculusUserNode(Node* parent);
		~OculusUserNode() = default;

		std::string ToString(bool verbose, uint depth) const override;

		OculusHeadNode* GetHead() const { return m_head; }

	public:
		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
		bool LoadChildrenFromXML(const tinyxml2::XMLElement* xmlData) override;
	};

}

#endif // OCULUSUSERNODE_H
