#pragma once

#include "ecs_universe/ComponentDetail.h"
#include "ecs_universe/Entity.h"

#define COMP_DIRTABLE                                                                                                  \
	struct Dirty {                                                                                                     \
	}

#define COMP_CREATEDESTROY                                                                                             \
	struct Create {                                                                                                    \
	};                                                                                                                 \
	struct Destroy {                                                                                                   \
	}

struct DirtyMovedComp {
};

struct DirtySrtComp {
};

struct BasicComponent {
	friend class ComponentsDb;

	std::string name;
	Entity self;


	void UpdateWorldTransforms();

	//
	Entity parent;
	Entity firstChild;

	Entity next;
	Entity prev;

	// Also moves children, handles moving ourselves under a child (by promoting all children).
	// NOTE: Be careful when you call this from loops
	void SetParent(Entity newParent = {}, bool preserveWorldTransform = true, int32 index = -1);

	void MarkDirtySrt();
	void MarkDirtyMoved();

	[[nodiscard]] const TransformCache& local() const { return local_; }
	[[nodiscard]] const TransformCache& world() const { return world_; }

private:
	TransformCache local_;
	TransformCache world_;

	void DetachFromParent();

	// Returns false if possibleChild == self
	bool IsDistantChild(Entity possibleChild);

public:
	void SetNodeTransformWCS(const glm::mat4& newWorldMatrix);


	// void SetNodePositionLCS(glm::vec3 lt);
	// void SetNodeOrientationLCS(glm::quat lo);
	//// in degrees / pitch, yaw, roll
	// void SetNodeEulerAnglesLCS(glm::vec3 pyr);
	// void SetNodeScaleLCS(glm::vec3 ls);

	void SetNodeTransformLCS(const glm::mat4& lm);
	// void SetNodeLookAtLCS(glm::vec3 lookAt);

	// void RotateNodeAroundAxisLCS(glm::vec3 localAxis, float degrees);
	// void AddNodePositionOffsetLCS(glm::vec3 offset);

	// void SetNodePositionWCS(glm::vec3 wt);
	// void SetNodeOrientationWCS(glm::quat wo);
	//// in degrees / pitch, yaw, roll
	// void SetNodeEulerAnglesWCS(glm::vec3 pyr);
	// void SetNodeScaleWCS(glm::vec3 ws);
	// void SetNodeTransformWCS(const glm::mat4& newWorldMatrix);
	// void SetNodeLookAtWCS(glm::vec3 lookAt);

	// void RotateNodeAroundAxisWCS(glm::vec3 worldAxis, float degrees);
	// void AddNodePositionOffsetWCS(glm::vec3 offset);
};
