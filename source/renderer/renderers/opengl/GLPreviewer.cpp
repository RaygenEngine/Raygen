#include "pch/pch.h"

#include "platform/windows/Win32Window.h"
#include "renderer/renderers/opengl/GLPreviewer.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "asset/AssetManager.h"
#include "system/Logger.h"

namespace ogl {

GLPreviewer::Preview::Preview(GLuint handle, const std::string& name, const std::string& description)
	: handle(handle)
	, name(name)
	, description(description)
{
}

void GLPreviewer::InitPreviewShaders(GLAssetManager* contextManager)
{
	m_simpleShader = contextManager->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/QuadWriteTexture.json");
}


void GLPreviewer::AddPreview(GLuint handle, const std::string& name, const std::string& description)
{
	if (!glIsTexture(handle)) {
		LOG_WARN("GLPreviewer: preview with name {}, is not memory resident, ingoring", name);
		return;
	}

	m_previews.emplace_back(handle, name, description);
}

void GLPreviewer::RenderPreview()
{
	if (m_previews.empty()) {
		LOG_WARN("GLPreviewer: no previews are memory resident anymore");
		return;
	}

	const auto preview = m_previews.at(m_currentPreview);

	auto wnd = Engine::GetMainWindow();
	glViewport(0, 0, wnd->GetWidth(), wnd->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(m_simpleShader->programId);

	m_simpleShader->SendTexture(preview.handle, 0);

	// big triangle trick, no vao
	report_glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLPreviewer::PreviousPreview()
{
	if (m_previews.empty()) {
		LOG_WARN("GLPreviewer: no previews are memory resident anymore");
		return;
	}

	const auto prev = m_currentPreview;
	m_currentPreview = !m_currentPreview ? m_previews.size() - 1 : m_currentPreview - 1;

	if (!glIsTexture(m_previews.at(m_currentPreview).handle)) {
		LOG_WARN("GLPreviewer: preview '{}', is not memory resident anymore", m_previews[m_currentPreview].name);

		m_previews.erase(m_previews.begin() + m_currentPreview);

		// reset
		m_currentPreview = prev;

		return PreviousPreview();
	}

	LOG_REPORT("GLPreviewer: previewing '{}'", m_previews.at(m_currentPreview).name);
}

void GLPreviewer::NextPreview()
{
	if (m_previews.empty()) {
		LOG_WARN("GLPreviewer: no previews are memory resident anymore");
		return;
	}

	const auto prev = m_currentPreview;
	++m_currentPreview %= m_previews.size();

	if (!glIsTexture(m_previews.at(m_currentPreview).handle)) {
		LOG_WARN("GLPreviewer: preview '{}', is not memory resident anymore", m_previews[m_currentPreview].name);

		m_previews.erase(m_previews.begin() + m_currentPreview);

		// reset
		m_currentPreview = prev;

		return NextPreview();
	}

	LOG_REPORT("GLPreviewer: previewing '{}'", m_previews.at(m_currentPreview).name);
}

} // namespace ogl
