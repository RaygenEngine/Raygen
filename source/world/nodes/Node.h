#pragma once

#include "reflection/GenMacros.h" // include gen macros here even if not needed to propagate to all node headers
#include "system/Object.h"
#include "core/MathAux.h"

#include <bitset>

class AssetManager;
class World;
class RootNode;

class Node;

using DirtyFlagset = std::bitset<64>;
using NodeDeleterFunc = void (*)(Node*);
using NodeUniquePtr = std::unique_ptr<Node, NodeDeleterFunc>;

// Casts a generic node to T without dynamic_cast, using the underlying reflection for checks
// Aborts if node was not of this type.
// Results in undefined behavior on configurations where CLOG_ABORT is disabled.
template<typename T>
T* NodeCast(Node* node)
{
	static_assert(std::is_base_of_v<Node, T>, "Template argument was not a node. Cast would always fail.");
	CLOG_ABORT(!node->IsA<T>(), "NodeCast failed. Given node: {}> {} was not a {}", node->GetClass().GetName(),
		node->GetName(), refl::GetName<T>());
	return static_cast<T*>(node);
}


class Node : public Object {
	//
	// REFLECTION BASE
	//
	// Stuff required and used by reflection that are specific to base node class.
	// This is similar to the macro code-gen but with a few changes for the base class.
	[[nodiscard]] static ReflClass& Z_MutableClass()
	{
		static ReflClass cl = ReflClass::Generate<Node>();
		return cl;
	}

public:
	using Parent = Node;

	[[nodiscard]] virtual const ReflClass& GetClass() const { return Node::StaticClass(); }

	[[nodiscard]] virtual const ReflClass& GetParentClass() const { return Parent::StaticClass(); }

	[[nodiscard]] static const ReflClass& StaticClass() { return Z_MutableClass(); }

private:
	friend class ReflClass;
	static void GenerateReflection(ReflClass& refl)
	{
		// Node class has no base variables
	}


public:
	// Base for dirty flagsets, use the macros. DOC
	struct DF {
		enum
		{
			// scaled, rotated or translated
			SRT,
			Hierarchy,
			Children,
			Properties,
			Created,

			_COUNT
		};
	};


private:
	// local
	glm::vec3 m_localPosition{ 0.f };
	glm::quat m_localOrientation{ glm::identity<glm::quat>() };
	glm::vec3 m_localScale{ 1.f };
	glm::mat4 m_localTransform{ glm::identity<glm::mat4>() };

	// world
	glm::vec3 m_position{ 0.f };
	glm::quat m_orientation{ glm::identity<glm::quat>() };
	glm::vec3 m_scale{ 1.f };
	glm::mat4 m_transform{ glm::identity<glm::mat4>() };

	DirtyFlagset m_dirty{};

	Node* m_parent{ nullptr };

	std::vector<NodeUniquePtr> m_children;

protected:
	std::string m_name;

	// TODO: keep only the local space one
	// currently world space
	math::AABB m_aabb{};
	math::AABB m_localBB{ { -0.3, -0.3, -0.3 }, { 0.3, 0.3, 0.3 } };
	virtual void CalculateWorldAABB();

private:
	// Dirty Functions
	void CallDirtyUpdate() { DirtyUpdate(m_dirty); };

	friend class Editor;
	friend class World;
	friend class NodeFactory;

	void AutoUpdateTransforms();

public:
	virtual ~Node() = default;

	[[nodiscard]] glm::vec3 GetNodePositionLCS() const { return m_localPosition; }
	[[nodiscard]] glm::quat GetNodeOrientationLCS() const { return m_localOrientation; }
	// pitch, yaw, roll
	[[nodiscard]] glm::vec3 GetNodeEulerAnglesLCS() const { return glm::degrees(glm::eulerAngles(m_localOrientation)); }
	[[nodiscard]] glm::vec3 GetNodeScaleLCS() const { return m_localScale; }
	[[nodiscard]] glm::mat4 GetNodeTransformLCS() const { return m_localTransform; }

	[[nodiscard]] glm::vec3 GetNodeUpLCS() const { return GetNodeOrientationLCS() * glm::vec3(0.f, 1.f, 0.f); }
	[[nodiscard]] glm::vec3 GetNodeRightLCS() const { return GetNodeOrientationLCS() * glm::vec3(1.f, 0.f, 0.f); }
	[[nodiscard]] glm::vec3 GetNodeForwardLCS() const { return GetNodeOrientationLCS() * glm::vec3(0.f, 0.f, -1.f); }

	[[nodiscard]] glm::vec3 GetNodePositionWCS() const { return m_position; }
	[[nodiscard]] glm::quat GetNodeOrientationWCS() const { return m_orientation; }
	// pitch, yaw, roll
	[[nodiscard]] glm::vec3 GetNodeEulerAnglesWCS() const { return glm::degrees(glm::eulerAngles(m_orientation)); }
	[[nodiscard]] glm::vec3 GetNodeScaleWCS() const { return m_scale; }
	[[nodiscard]] glm::mat4 GetNodeTransformWCS() const { return m_transform; }

	[[nodiscard]] glm::vec3 GetNodeUpWCS() const { return GetNodeOrientationWCS() * glm::vec3(0.f, 1.f, 0.f); }
	[[nodiscard]] glm::vec3 GetNodeRightWCS() const { return GetNodeOrientationWCS() * glm::vec3(1.f, 0.f, 0.f); }
	[[nodiscard]] glm::vec3 GetNodeForwardWCS() const { return GetNodeOrientationWCS() * glm::vec3(0.f, 0.f, -1.f); }

	[[nodiscard]] bool IsLeaf() const { return m_children.empty(); }
	[[nodiscard]] const std::string& GetName() const { return m_name; };
	[[nodiscard]] const std::vector<NodeUniquePtr>& GetChildren() const { return m_children; }

	[[nodiscard]] DirtyFlagset GetDirtyFlagset() const { return m_dirty; }

	// Returns nullptr IF AND ONLY IF "this" node is the root node.
	[[nodiscard]] Node* GetParent() const { return m_parent; }
	[[nodiscard]] bool IsRoot() const { return m_parent == nullptr; }
	[[nodiscard]] RootNode* GetWorldRoot() const;

	[[nodiscard]] math::AABB GetAABB() const { return m_aabb; }

	void SetNodePositionLCS(glm::vec3 lt);
	void SetNodeOrientationLCS(glm::quat lo);
	// in degrees / pitch, yaw, roll
	void SetNodeEulerAnglesLCS(glm::vec3 pyr);
	void SetNodeScaleLCS(glm::vec3 ls);
	void SetNodeTransformLCS(const glm::mat4& lm);
	void SetNodeLookAtLCS(glm::vec3 lookAt);

	void RotateNodeAroundAxisLCS(glm::vec3 localAxis, float degrees);
	void AddNodePositionOffsetLCS(glm::vec3 offset);

	void SetNodePositionWCS(glm::vec3 wt);
	void SetNodeOrientationWCS(glm::quat wo);
	// in degrees / pitch, yaw, roll
	void SetNodeEulerAnglesWCS(glm::vec3 pyr);
	void SetNodeScaleWCS(glm::vec3 ws);
	void SetNodeTransformWCS(const glm::mat4& newWorldMatrix);
	void SetNodeLookAtWCS(glm::vec3 lookAt);

	void RotateNodeAroundAxisWCS(glm::vec3 worldAxis, float degrees);
	void AddNodePositionOffsetWCS(glm::vec3 offset);

	void SetName(const std::string& name) { m_name = name; }
	void DeleteChild(Node* child);
	//
	// LOADING
	//

	// Runs late in the frame, only on nodes that at least one m_dirty is set.
	virtual void DirtyUpdate(DirtyFlagset dirtyFlags){};

	virtual void Update(float deltaSeconds){};

	// cache world transform top down (and where needed to be updated)
	void UpdateTransforms(const glm::mat4& parentMatrix);

	void SetDirty(uint32 flagIndex) { m_dirty.set(flagIndex); }
	void SetDirtyMultiple(DirtyFlagset other) { m_dirty |= other; };

	template<typename T>
	bool IsA()
	{
		if constexpr (std::is_same_v<T, Node>) {
			return true;
		}

		auto nodeClass = &GetClass();
		while (nodeClass != &Node::StaticClass()) {
			if (nodeClass == &T::StaticClass()) {
				return true;
			}
			nodeClass = nodeClass->GetParentClass();
		}
		return false;
	}

protected:
	// Can accept node types that are not registered to the factory
	// This will not fetch subclasses but will only match exact classes, name is case sensitive
	// The expected use of this function is to be used on Dirty DF::Children to grab actual node pointers that you can
	// safely store as members. You can be sure that the pointers will always be valid during Update()
	//
	// Any other use may cause iterator invalidation or other undefined behavior.
	//
	// NOTE: requires includes: World.h
	template<typename T>
	T* GetOrCreateChild(const std::string& name)
	{
		for (auto& childUnq : m_children) {
			Node* child = childUnq.get();
			if (child->GetClass() == T::StaticClass() && child->GetName() == name) {
				return static_cast<T*>(child);
			}
		}

		return Engine::GetWorld()->CreateNode<T>(name, this);
	}
};
