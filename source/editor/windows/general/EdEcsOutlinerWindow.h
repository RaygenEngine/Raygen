#pragma once
#include "editor/windows/EdWindow.h"
#include "ecs_universe/Entity.h"


class ECS_World;
namespace ed {
class EcsOutlinerWindow : public UniqueWindow {

	enum class RenameStatus
	{
		Inactive,
		FirstFrame,
		OtherFrames
	} m_renameStatus{ RenameStatus::Inactive };
	int32 m_renameFrame{ -1 };
	std::string m_renameString{};


	std::function<void()> postIterCommand;

public:
	EcsOutlinerWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void ImguiDraw();
	void Run_ContextPopup(ECS_World& world, Entity entity);
	void Run_SpaceContextPopup(ECS_World& world);
	// void Run_NewNodeMenu(Node* underNode);
	void Run_OutlinerDropEntity(Entity entity);
	virtual ~EcsOutlinerWindow() = default;

	void DrawRecurseEntity(ECS_World& world, Entity ent, int32 depth = 0);

	static inline Entity selected;
};
} // namespace ed