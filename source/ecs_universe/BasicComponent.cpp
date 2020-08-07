#include "pch.h"
#include "BasicComponent.h"


#include <glm/gtx/matrix_decompose.hpp>


void BasicComponent::SetParent(Entity newParent, int32 index)
{
	if (newParent == self) {
		return;
	}

	if (IsDistantChild(newParent)) {
		// If new parent is distant child, we cannot directly move under it because we will detach from the tree
		// We will first promote our children to the parent node, then move ourselves under them
		auto ourOriginalParent = parent;

		DetachFromParent();

		auto child = firstChild;
		while (child) {
			auto nextChild = child->next;
			child->SetParent(ourOriginalParent);
			child = nextChild;
		}

		// Continue into SetParent normally...
	}

	// Stuff to be done when changing parent:

	// REMOVE:
	//	Remove from siblings list
	//	Update firstChild of parent if required
	//
	DetachFromParent();


	// ADD:
	//	Add to child list index
	//  Update firstChild of parent if required
	// index == -1 means at the ending of the list

	if (!newParent) {
		UpdateWorldTransforms();
		return;
	}

	auto current = newParent->firstChild;
	Entity previous;

	int32 count = 0;
	while (current) {
		if (count == index) {
			// Insert here
			break;
		}
		count++;
		previous = current;
		current = current->next;
	}

	// Insert BEFORE "current"

	auto insertAfter = previous;
	auto insertBefore = current;

	if (!insertAfter) {
		newParent->firstChild = self;
		parent = newParent;
		UpdateWorldTransforms();

		return;
	}

	insertAfter->next = self;

	if (insertBefore) {
		insertBefore->prev = self;
	}

	next = insertBefore;
	prev = insertAfter;
	parent = newParent;

	UpdateWorldTransforms();
}

void BasicComponent::MarkDirtySrt()
{
	self.registry->get_or_emplace<DirtySrtComp>(self.entity);
}

void BasicComponent::MarkDirtyMoved()
{
	self.registry->get_or_emplace<DirtyMovedComp>(self.entity);
	MarkDirtySrt();
}

void BasicComponent::DetachFromParent()
{
	if (!parent) {
		CLOG_ERROR(next || prev, "Corrupt entity relationship found");
		return;
	}

	// Remove from siblings list
	if (next) {
		next->prev = prev;
	}
	if (prev) {
		prev->next = next;
	}

	// Update parent first if required
	if (parent && parent->firstChild == self) {
		parent->firstChild = next;
	}
	parent = {};
}

bool BasicComponent::IsDistantChild(Entity possibleChild)
{
	if (possibleChild == self) {
		return false;
	}
	// Go up the tree until you reach root or find self
	auto current = possibleChild;
	while (current) {
		if (current == self) {
			return true;
		}
		current = current->parent;
	}
	return false;
}


void BasicComponent::SetNodeTransformWCS(const glm::mat4& newWorldMatrix)
{
	auto parentMatrix = parent ? parent->world().transform : glm::identity<glm::mat4>();
	world_.transform = newWorldMatrix;

	SetNodeTransformLCS(glm::inverse(parentMatrix) * newWorldMatrix);
}

void BasicComponent::SetNodeTransformLCS(const glm::mat4& lm)
{
	local_.transform = lm;
	local_.Decompose();

	MarkDirtyMoved();
	UpdateWorldTransforms();
}


void BasicComponent::UpdateWorldTransforms()
{
	MarkDirtySrt();
	local_.Compose();

	auto current = firstChild;
	world_.transform = parent ? parent->world_.transform * local_.transform : local_.transform;

	world_.Decompose();

	while (current) {
		current->UpdateWorldTransforms();
		current = current->next;
	}
}

void BasicComponent::TransformCache::Compose()
{
	transform = math::transformMat(scale, orientation, position);
}

void BasicComponent::TransformCache::Decompose()
{
	glm::vec4 persp{};
	glm::vec3 skew{};

	glm::decompose(transform, scale, orientation, position, skew, persp);
}