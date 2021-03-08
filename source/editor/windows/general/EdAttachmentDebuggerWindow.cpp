#include "EdAttachmentDebuggerWindow.h"

#include "editor/EditorObject.h"
#include "engine/Events.h"
#include "rendering/Layer.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneSpotlight.h"


namespace ed {

AttachmentDebuggerWindow::AttachmentDebuggerWindow(std::string_view name)
	: ed::UniqueWindow(name)
{
	Event::OnViewportUpdated.BindFlag(this, m_willInvalidateDescriptors);
}

void AttachmentDebuggerWindow::ImguiDraw() {}

} // namespace ed
