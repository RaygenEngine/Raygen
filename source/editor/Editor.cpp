#include "pch.h"
#include "Editor.h"

#include "editor/EditorObject.h"

void Editor::Init()
{
	EditorObj = new EditorObject();
}

void Editor::Destroy()
{
	delete EditorObj;
}

void Editor::Update()
{
	EditorObj->UpdateEditor();
}

bool Editor::ShouldUpdateWorld()
{
	return EditorObj->ShouldUpdateWorld();
}
