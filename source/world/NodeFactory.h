#pragma once

#include "nodes/Node.h"
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace detail {
constexpr std::string_view filter = "Node";
} // namespace detail


class NodeFactory : public Object {

	struct NodeClassEntry {
		const ReflClass* classPtr;
		std::function<Node*()> newInstance;
	};

	std::unordered_map<std::string, NodeClassEntry> m_nodeEntries;

	friend class World;

protected:
	std::string FilterNodeName(std::string_view v);

	template<typename T>
	void RegisterNode()
	{
		static_assert(std::is_base_of_v<Node, T> && !std::is_same_v<Node, T>, "You can only register Node subclasses");

		std::string name{ FilterNodeName(refl::GetName<T>()) };

		NodeClassEntry entry;
		entry.classPtr = &T::StaticClass();
		entry.newInstance = &T::NewInstance;

		m_nodeEntries.insert({ smath::ToLower(name), entry });
	}

	template<typename... Nodes>
	void RegisterNodeList()
	{
		(RegisterNode<Nodes>(), ...);
	}

	Node* NewNodeFromType(const std::string& type);


public:
	NodeFactory() { RegisterNodes(); }
	// Loads all nodes from the xml code as children to 'parentNode'.
	void LoadNodeAndChildren(const nlohmann::json& jsonData, Node* parentNode);

	void LoadChildren(const nlohmann::json& jsonArray, Node* parent);

	virtual void RegisterNodes();

private:
	// Utility helpers for loading
	void LoadNode_Trs(const nlohmann::json& jsonTrsObject, Node* nodeToLoadInto);
	void LoadNode_Properties(const nlohmann::json& j, Node* node);
};
