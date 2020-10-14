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
	EditorObject->UpdateEditor();
}
