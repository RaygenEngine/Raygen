#include "Editor.h"

#include "editor/EditorObject.h"
#include "editor/windows/general/EdOutlinerWindow.h"
#include "editor/imgui/ImguiImpl.h"

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

void Editor::SetSelection(Entity ent)
{
	ed::OutlinerWindow::selected = ent;
}

void Editor::ClearSelection()
{
	SetSelection({});
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

void Editor::RecordCmd(vk::CommandBuffer* drawCommandBuffer)
{
	ImguiImpl::RenderVulkan(drawCommandBuffer);
}

std::pair<glm::vec2, glm::vec2> Editor::GetIconUV(const char* icon)
{
	return ImguiImpl::GetIconUV(icon);
}

ImTextureID Editor::GetFontIconTexture()
{
	return ImguiImpl::GetFontIconTexture();
}
