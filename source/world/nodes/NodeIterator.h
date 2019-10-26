#pragma once

// A basic (non stl compliant) iterator that will iterate over all nodes of type T or subtypes in the world.
// the order of the nodes is undefined
// Hacky implementation, iterator contains all the info required, and end iterator is just a nullptr
class Node;

template<typename NodeTypeT>
struct NodeIterator {
	using NodeType = NodeTypeT;

	std::unordered_set<Node*>::iterator underlyingIterator;

public:
	NodeIterator(std::unordered_set<Node*>::iterator it)
		: underlyingIterator(it)
	{
	}

	bool operator!=(NodeIterator<NodeTypeT>& rhs) { return underlyingIterator != rhs.underlyingIterator; }

	NodeType* operator*()
	{
		Node* node = *underlyingIterator;
		return NodeCast<NodeType>(node);
	}

	void operator++() { ++underlyingIterator; }
};

// Adding or removing nodes instantly invalidates this iterator
template<typename NodeTypeT>
struct NodeIterable {
	using NodeType = NodeTypeT;

private:
	// PERF: possible to iterate over each set individually without copy
	std::unordered_set<Node*> setToIterate;

	void AddSet(const std::unordered_set<Node*>& toAdd) { setToIterate.insert(toAdd.begin(), toAdd.end()); }

public:
	NodeIterable(std::unordered_map<size_t, std::unordered_set<Node*>>& mapToNodes)
	{
		const ReflClass& cl = NodeType::StaticClass();

		AddSet(mapToNodes[cl.GetTypeId().hash()]);
		for (auto& childClass : cl.GetChildClasses()) {
			AddSet(mapToNodes[childClass->GetTypeId().hash()]);
		}
	}

	NodeIterator<NodeType> begin() { return NodeIterator<NodeType>(setToIterate.begin()); }
	NodeIterator<NodeType> end() { return NodeIterator<NodeType>(setToIterate.end()); }
};
