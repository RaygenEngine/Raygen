#pragma once
#include "editor/windows/EdWindow.h"
#include "ecs_universe/Entity.h"

class Node;

namespace ed {
class EcsOutlinerWindow : public UniqueWindow {
public:
	EcsOutlinerWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void ImguiDraw();
	// bool Run_ContextPopup(Node* node);
	// void Run_NewNodeMenu(Node* underNode);
	// void Run_OutlinerDropTarget(Node* node);
	virtual ~EcsOutlinerWindow() = default;

	static inline Entity selected;
};
} // namespace ed
