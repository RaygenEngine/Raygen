#pragma once
#include "engine/Listener.h"

#include <nlohmann/json_fwd.hpp>
#include <functional>
#include <map>

class Node;

class NodeFactory : public Listener {


	// PERF: use unordered map here. Ordered map enables the the editor to show the list alphabetically.
	std::map<std::string, const class ReflClass*> m_nodeEntries;

	friend class World;
	friend class EditorObject_;


public:
	NodeFactory() { RegisterNodes(); }
	// Loads all nodes from the xml code as children to 'parentNode'.
	void LoadNodeAndChildren(const nlohmann::json& jsonData, Node* parentNode);

	void LoadChildren(const nlohmann::json& jsonArray, Node* parent);

	virtual void RegisterNodes();


	// Avoid using this, this does NOT register absolutely anything. It just returns a new instance.
	Node* NewNodeFromType(const std::string& type);

	// Avoid using this too, this does NOT register absolutely anything. It just returns a new instance.
	template<typename T>
	static T* NewNode()
	{
		return static_cast<T*>(T::NewInstance());
	}

	auto Z_GetEntries() { return m_nodeEntries; }

private:
	// Utility helpers for loading
	void LoadNode_Trs(const nlohmann::json& jsonTrsObject, Node* nodeToLoadInto);
	void LoadNode_Properties(const nlohmann::json& j, Node* node);
};
