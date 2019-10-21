#pragma once

#include "asset/AssetPod.h"
#include <functional>
class Node;

struct NodeContextActions {
	using FuncType = std::function<void(Node*)>;
	NodeContextActions();

	struct Entry {
		const char* name; // if name is nullptr, this is a spliter
		FuncType function;

		Entry()
			: name(nullptr)
		{
		}

		Entry(const char* inName, FuncType inFunc, bool button = true)
			: name(inName)
			, function(inFunc)
		{
		}


		bool IsSplitter() const { return name == nullptr; }
	};
	std::vector<Entry> GetActions(Node* node, bool extendedList = false);

private:
	std::vector<Entry> baseActions;
};
