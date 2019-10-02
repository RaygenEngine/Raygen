#pragma once

#include "world/nodes/user/UserNode.h"
#include "world/nodes/camera/CameraNode.h"


class FreeformUserNode : public UserNode
{
	REFLECTED_NODE(FreeformUserNode, UserNode) 
	{
		REFLECT_VAR(m_int);
	}

	CameraNode* m_camera;
	int32 m_int{ 1 };
public:
	FreeformUserNode(Node* parent);
	~FreeformUserNode() = default;

	std::string ToString(bool verbose, uint depth) const override;
		
	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
	bool PostChildrenLoaded() override;

	CameraNode* GetCamera() const { return m_camera; }

	void Update(float deltaTime) override;

	void ToString(std::ostream& os) const override { os << "node-type: FreeformUserNode, name: " << GetName(); }
};

