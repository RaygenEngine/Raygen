#pragma once
#include "editor/windows/EdWindow.h"

namespace ed {
class AttachmentDebuggerWindow : public ed::UniqueWindow {
	BoolFlag m_willInvalidateDescriptors;
	glm::ivec2 m_imgSize{ 256, 256 };
	std::unordered_map<std::string, bool> isAttachmentOpen;

public:
	inline static const char* Category = "Debug";

	AttachmentDebuggerWindow(std::string_view name);


	virtual void ImguiDraw();
	virtual ~AttachmentDebuggerWindow() = default;
};

} // namespace ed
