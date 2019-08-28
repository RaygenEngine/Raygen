#pragma once

#include "system/EngineObject.h"

#include "tinyxml2/tinyxml2.h"

namespace World
{
	class Node : public System::EngineObject
	{
		// local
		glm::vec3 m_localTranslation;
		glm::quat m_localOrientation;
		glm::vec3 m_localScale;
		glm::mat4 m_localMatrix;

		// world
		glm::vec3 m_worldTranslation;
		glm::quat m_worldOrientation;
		glm::vec3 m_worldScale;
		glm::mat4 m_worldMatrix;

		bool m_dirty;
		bool m_updateLocalMatrix;

	protected:
		std::string m_name;

		Node* m_parent;

		// for now ownership is given to parent nodes (later on, world should be manager) 
		std::vector<std::shared_ptr<Node>> m_children;

	public:
		// World initializes with Engine
		Node(System::Engine* engine);
		// Nodes have pObject = parentNode->GetWorld() = World
		Node(Node* pNode);
		virtual ~Node() = default;

		glm::vec3 GetLocalTranslation() const { return m_localTranslation; }
		glm::quat GetLocalOrientation() const { return m_localOrientation; }
		glm::vec3 GetLocalScale() const { return m_localScale; }		
		glm::mat4 GetLocalMatrix() const { return m_localMatrix; }

		void SetLocalTranslation(const glm::vec3& lt);
		void SetLocalOrientation(const glm::quat& lo);
		void SetLocalScale(const glm::vec3& ls);
		void SetLocalMatrix(const glm::mat4& lm);

		glm::vec3 GetWorldTranslation() const { return m_worldTranslation; }
		glm::quat GetWorldOrientation() const { return m_worldOrientation; }
		glm::vec3 GetWorldScale() const { return m_worldScale; }
		glm::mat4 GetWorldMatrix() const { return m_worldMatrix; }

		glm::vec3 GetUp() const { return GetWorldOrientation() * glm::vec3(0.f, 1.f, 0.f); }
		glm::vec3 GetRight() const { return GetWorldOrientation() * glm::vec3(1.f, 0.f, 0.f); }
		glm::vec3 GetFront() const { return GetWorldOrientation() * glm::vec3(0.f, 0.f, -1.f); }

		bool IsLeaf() const { return m_children.empty(); }
		std::string GetName() const { return m_name; }

		void AddChild(std::shared_ptr<Node> child) { m_children.emplace_back(child); }
		
		bool LoadFromXML(const tinyxml2::XMLElement* xmlData);

		virtual bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData);
		virtual bool LoadChildrenFromXML(const tinyxml2::XMLElement* xmlData);

		// mark dirty self and children
		void MarkDirty();
		// cache world transform bottom up (and where needed to be updated)
		virtual void CacheWorldTransform();

		void Move(const glm::vec3& direction, float magnitude);
		void MoveUp(float magnitude);
		void MoveDown(float magnitude);
		void MoveRight(float magnitude);
		void MoveLeft(float magnitude);
		void MoveFront(float magnitude);
		void MoveBack(float magnitude);

		// not tested
		void Orient(float yaw, float pitch, float roll);

		void OrientWithoutRoll(float yaw, float pitch);

		void OrientYaw(float yaw);
		
	protected:
		virtual std::string ToString(bool verbose, uint depth) const;

	public:

		virtual void ToString(std::ostream& os) const { os << "object-type: Node, id: " << GetObjectId(); }
	};
}

