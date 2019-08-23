#ifndef OCULUSHEADNODE_H
#define OCULUSHEADNODE_H

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"

namespace World
{

	class OculusHeadNode : public Node
	{
		friend class OculusUserNode;

		CameraNode* m_eyes[ET_COUNT];

	public:
		OculusHeadNode(Node* parent);
		~OculusHeadNode() = default;

		std::string ToString(bool verbose, uint depth) const override;

		CameraNode* GetEye(EyeTarget index) const { return m_eyes[index]; }

	public:
		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
		Node* LoadSpecificChild(const tinyxml2::XMLElement* xmlChildData) override;
		bool PostChildrenLoaded() override;
	};

}

#endif // OCULUSHEADNODE_H
