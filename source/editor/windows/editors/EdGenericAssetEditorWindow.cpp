#include "EdGenericAssetEditorWindow.h"

#include "editor/imgui/ImEd.h"
// HACK:
// GenericImguiDrawEntry is implemented in EdPropertyEditorWindow.cpp to avoid duplicating the Reflection To Imgui
// Visitor.

void ed::GenericAssetEditorWindow::ImguiDraw()
{
	DrawSaveButton();
	GenericImguiDrawEntry(entry);
}
