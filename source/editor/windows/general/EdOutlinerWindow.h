#pragma once
#include "editor/windows/EdWindow.h"

class Node;

namespace ed {
class OutlinerWindow : public UniqueWindow {
public:
	OutlinerWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void ImguiDraw();
	bool Run_ContextPopup(Node* node);
	void Run_NewNodeMenu(Node* underNode);
	void Run_OutlinerDropTarget(Node* node);
	virtual ~OutlinerWindow() = default;
};
} // namespace ed
