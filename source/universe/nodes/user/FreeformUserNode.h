#pragma once
#include "universe/nodes/user/UserNode.h"

class FreeformUserNode : public UserNode {
	REFLECTED_NODE(FreeformUserNode, UserNode) {}

public:
	void Update(float deltaTime) override;
};
