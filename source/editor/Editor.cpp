#include "pch.h"
#include "Editor.h"

#include "editor/EditorObject.h"
#include "editor/windows/general/EdEcsOutlinerWindow.h"

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

	return ed::EcsOutlinerWindow::selected;
}

void Editor::Update()
{
	EditorObject->UpdateEditor();
}
