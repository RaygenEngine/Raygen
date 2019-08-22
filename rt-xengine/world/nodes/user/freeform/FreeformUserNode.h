#ifndef FREEFORMUSERNODE_H
#define FREEFORMUSERNODE_H

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/user/UserNode.h"


namespace World
{

	class FreeformUserNode : public UserNode
	{
		CameraNode* m_camera;

	public:
		FreeformUserNode(Node* parent);
		~FreeformUserNode() = default;

		std::string ToString(bool verbose, uint depth) const override;

		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
		bool LoadChildrenFromXML(const tinyxml2::XMLElement* xmlData) override;

		CameraNode* GetCamera() const { return m_camera; }

		void Update() override;
	};

}

#endif // FREEFORMUSERNODE_H
