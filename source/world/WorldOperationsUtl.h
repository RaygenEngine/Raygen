#pragma once
#include "world/nodes/Node.h"

// TODO:
struct worldop {

	static void MoveChildUp(Node* node);
	static void MoveChildDown(Node* node);

	static void MoveChildOut(Node* node);
	static void MakeChildOf(Node* newParent, Node* node);
	static void MakeActiveCamera(Node* node);
};
