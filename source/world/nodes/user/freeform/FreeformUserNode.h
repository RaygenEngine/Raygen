#pragma once

#include "world/nodes/user/UserNode.h"
#include "world/nodes/camera/CameraNode.h"


class FreeformUserNode : public UserNode {
	REFLECTED_NODE(FreeformUserNode, UserNode) {}

public:
	void Update(float deltaTime) override;
};
