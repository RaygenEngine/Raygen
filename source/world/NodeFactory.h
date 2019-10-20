#pragma once

#include "nodes/Node.h"
#include "asset/util/ParsingAux.h"
#include "core/StringAux.h"

#include <functional>
#include <map>
#include <nlohmann/json.hpp>

class NodeFactory : public Object {

	struct NodeClassEntry {
		const ReflClass* classPtr;
		std::function<Node*()> newInstance;
	};

	// PERF: use unordered map here. Ordered map enables the the editor to show the list alphabetically.
	std::map<std::string, NodeClassEntry> m_nodeEntries;

	friend class World;
	friend class Editor;

protected:
	template<typename T>
	void RegisterNode()
	{
		static_assert(std::is_base_of_v<Node, T> && !std::is_same_v<Node, T>, "You can only register Node subclasses");

		std::string name{ sceneconv::FilterNodeClassName(refl::GetName<T>()) };

		NodeClassEntry entry;
		entry.classPtr = &T::StaticClass();
		entry.newInstance = &T::NewInstance;

		m_nodeEntries.insert({ name, entry });
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


	// Avoid using this, this does NOT register absolutely anything. It just returns a new instance.
	template<typename T>
	static T* NewNode()
	{
		return static_cast<T*>(T::NewInstance());
	}

private:
	// Utility helpers for loading
	void LoadNode_Trs(const nlohmann::json& jsonTrsObject, Node* nodeToLoadInto);
	void LoadNode_Properties(const nlohmann::json& j, Node* node);
};
