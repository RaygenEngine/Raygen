#pragma once
#include "editor/windows/EdWindow.h"
#include "universe/Entity.h"


class EcsWorld;
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
	void Run_ContextPopup(EcsWorld& world, Entity entity);
	void Run_SpaceContextPopup(EcsWorld& world);
	// void Run_NewNodeMenu(Node* underNode);
	void Run_OutlinerDropEntity(Entity entity);
	virtual ~EcsOutlinerWindow() = default;

	void DrawRecurseEntity(EcsWorld& world, Entity ent, int32 depth = 0);

	static inline Entity selected;
};
} // namespace ed
