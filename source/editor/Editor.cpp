#include "Editor.h"

#include "editor/EditorObject.h"
#include "editor/windows/general/EdOutlinerWindow.h"

void Editor::Init()
{
	EditorObject = new EditorObject_();
}

void Editor::Destroy()
{
	delete EditorObject;
}

Entity Editor::GetSelection()
{
	return ed::OutlinerWindow::selected;
}


void Editor::Update()
{
	if (EditorObject) {
		EditorObject->UpdateEditor();
	}
}

void Editor::BeforePlayWorld(World& world)
{
	if (EditorObject) {
		EditorObject->BeforePlayWorld(world);
	}
}

void Editor::AfterStopWorld(World& world)
{
	if (EditorObject) {
		EditorObject->AfterStopWorld(world);
	}
}
