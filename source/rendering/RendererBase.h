#pragma once

#include "engine/Listener.h"

namespace vl {
class OutputPassBase;
struct RImage;
struct RFramebuffer;
struct RenderingPassInstance;
} // namespace vl

namespace vk {
struct ImageView;
struct CommandBuffer;
} // namespace vk

struct SceneRenderDesc;

namespace vl {
class RendererBase : public Listener {
public:
	struct AttachmentData {
		void* descSet;
		const char* name;
		glm::ivec2 extent;
	};

private:
	std::vector<AttachmentData> m_debugAttachments;

protected:
	// register attachment that will show in attachment debugger when this
	// renderer is active
	void RegisterDebugAttachment(RImage& attachment);
	void RegisterDebugAttachment(RFramebuffer& attachment);
	void RegisterDebugAttachment(RenderingPassInstance& attachment);
	void ClearDebugAttachments() { m_debugAttachments.clear(); }

public:
	virtual void ResizeBuffers(uint32 width, uint32 height) = 0;
	virtual void RecordCmd(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) = 0;
	virtual InFlightResources<vk::ImageView> GetOutputViews() const = 0;

	const std::vector<AttachmentData>& GetDebugAttachments() const { return m_debugAttachments; }

	virtual ~RendererBase() = default;
};
} // namespace vl
