#include "pch.h"
#include "WorldOperationsUtl.h"

#include "engine/Engine.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/Universe.h"

void worldop::MoveChildUp(Node* node)
{
	auto& children = node->GetParent()->m_children;

	auto thisIt = std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	// Should be impossible unless world ptrs are corrupted
	CLOG_ABORT(thisIt == end(children), "Attempting to move child not in its parent container.");

	if (thisIt != begin(children)) {
		std::iter_swap(thisIt, thisIt - 1);
	}
	node->SetDirty(Node::DF::Children);
}

void worldop::MoveChildDown(Node* node)
{
	auto& children = node->GetParent()->m_children;
	auto thisIt = std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	// Should be impossible unless world ptrs are corrupted
	CLOG_ABORT(thisIt == end(children), "Attempting to move child not in its parent container.");

	if (thisIt + 1 != end(children)) {
		std::iter_swap(thisIt, thisIt + 1);
	}
	node->SetDirty(Node::DF::Children);
}

void worldop::MoveChildOut(Node* node)
{
	if (node->GetParent()->IsRoot()) {
		return;
	}

	auto worldMatrix = node->GetNodeTransformWCS();
	node->GetParent()->SetDirty(Node::DF::Children);

	auto& children = node->GetParent()->m_children;
	std::vector<NodeUniquePtr>::iterator thisIt
		= std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	NodeUniquePtr src = std::move(*thisIt);

	children.erase(thisIt);

	Node* insertBefore = node->GetParent();
	auto& newChildren = insertBefore->GetParent()->m_children;

	auto insertAt = std::find_if(
		begin(newChildren), end(newChildren), [&insertBefore](auto& elem) { return insertBefore == elem.get(); });

	newChildren.emplace(insertAt, std::move(src));

	node->m_parent = insertBefore->GetParent();
	node->SetNodeTransformWCS(worldMatrix);
	node->SetDirty(Node::DF::Hierarchy);
	node->GetParent()->SetDirty(Node::DF::Children);
}


void worldop::MakeChildOf(Node* newParent, Node* node)
{
	if (!node || !newParent || node == newParent) {
		return;
	}

	// We cannot move a parent to a child. Start from node and iterate to root. If we find "selectedNode" we cannot
	// move.
	Node* pathNext = newParent->GetParent();
	while (pathNext) {
		if (pathNext == node) {
			LOG_INFO("Cannot move '{}' under '{}' because the second is a child of the first.", node->GetName(),
				newParent->GetName());
			return;
		}
		pathNext = pathNext->GetParent();
	}

	auto worldMatrix = node->GetNodeTransformWCS();
	node->GetParent()->SetDirty(Node::DF::Children); // That parent is losing a child.

	auto& children = node->GetParent()->m_children;
	std::vector<NodeUniquePtr>::iterator thisIt
		= std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	NodeUniquePtr src = std::move(*thisIt);

	children.erase(thisIt);

	newParent->m_children.emplace_back(std::move(src));

	node->m_parent = newParent;
	node->SetNodeTransformWCS(worldMatrix);
	node->SetDirty(Node::DF::Hierarchy);
	node->GetParent()->SetDirty(Node::DF::Children);
}

void worldop::MakeActiveCamera(Node* node)
{
	if (node->IsA<CameraNode>()) {
		Universe::MainWorld->SetActiveCamera(NodeCast<CameraNode>(node));
	}
}
