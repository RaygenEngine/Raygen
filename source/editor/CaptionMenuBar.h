#pragma once

#include "core/BoolFlag.h"
#include "editor/EdMenu.h"
#include "editor/EdOperation.h"
#include "editor/EditorCamera.h"


#include <memory>
#include <functional>


class EditorObject_;
namespace ed {
struct CaptionMenuBar : public Listener {
public:
	Menu mainMenu{ "MainMenu" };

	CaptionMenuBar(EditorObject_& parent);


	void Draw();

	CaptionMenuBar(const CaptionMenuBar&) = delete;
	CaptionMenuBar(CaptionMenuBar&&) = delete;
	CaptionMenuBar& operator=(const CaptionMenuBar&) = delete;
	CaptionMenuBar& operator=(CaptionMenuBar&&) = delete;


	void MakeMainMenu();

private:
	void DrawBar();

	BoolFlag openPopupDeleteLocal{ false };


	bool isMaximised{ false };

	EditorObject_& editor;
};
} // namespace ed
