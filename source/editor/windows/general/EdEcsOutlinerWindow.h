#pragma once
#include "editor/windows/EdWindow.h"
#include "ecs_universe/Entity.h"


class ECS_World;
namespace ed {
class EcsOutlinerWindow : public UniqueWindow {
public:
	EcsOutlinerWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void ImguiDraw();
	void Run_ContextPopup(ECS_World& world, Entity entity);
	// void Run_NewNodeMenu(Node* underNode);
	// void Run_OutlinerDropTarget(Node* node);
	virtual ~EcsOutlinerWindow() = default;

	void DrawRecurseEntity(ECS_World& world, Entity ent, int32 depth = 0);

	static inline Entity selected;
};
} // namespace ed
