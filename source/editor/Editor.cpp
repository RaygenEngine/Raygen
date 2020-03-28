#include "pch.h"
#include "Editor.h"

#include "editor/EditorObject.h"

void Editor::Init()
{
	EditorObject = new EditorObject_();
}

void Editor::Destroy()
{
	delete EditorObject;
}

void Editor::Update()
{
	EditorObject->UpdateEditor();
}

bool Editor::ShouldUpdateWorld()
{
	return EditorObject->ShouldUpdateWorld();
}
