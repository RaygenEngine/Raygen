#pragma once
#include "universe/nodes/Node.h"

struct worldop {

	static void MoveChildUp(Node* node);
	static void MoveChildDown(Node* node);

	static void MoveChildOut(Node* node);
	static void MakeChildOf(Node* newParent, Node* node);
	static void MakeActiveCamera(Node* node);
};
