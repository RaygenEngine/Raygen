#pragma once

#include "renderer/ObserverRenderer.h"
#include "renderer/GenericGpuAssetManager.h"
#include "system/Engine.h"
#include "renderer/renderers/opengl/GLUtil.h"

#define report_glDrawElements(mode, count, type, indices)                                                              \
	GLCheckError();                                                                                                    \
	auto dr = Engine::GetDrawReporter();                                                                               \
	++dr->draws;                                                                                                       \
	dr->tris += count / 3;                                                                                             \
	glDrawElements(mode, count, type, indices)

#define report_glDrawArrays(mode, first, count)                                                                        \
	GLCheckError();                                                                                                    \
	auto dr = Engine::GetDrawReporter();                                                                               \
	++dr->draws;                                                                                                       \
	dr->tris += count / 3;                                                                                             \
	glDrawArrays(mode, first, count)

namespace ogl {
class GLAssetManager;
class GLPreviewer;

class GLRendererBase : public ObserverRenderer {
	HWND m_assochWnd{ nullptr };
	HDC m_hdc{ nullptr };
	HGLRC m_hglrc{ nullptr };

	std::unique_ptr<GLAssetManager> m_glAssetManager;
	std::unique_ptr<GLPreviewer> m_glPreviewer;

	bool m_vsyncEnabled{ true };
	bool m_previewerEnabled{ false };

public:
	GLRendererBase();

	virtual ~GLRendererBase();

	virtual void InitScene() = 0;
	void Init(HWND assochWnd, HINSTANCE instance) final override;

	void SwapBuffers();

	[[nodiscard]] GLAssetManager* GetGLAssetManager() const { return m_glAssetManager.get(); }
	[[nodiscard]] GLPreviewer* GetGLPreviewer() const { return m_glPreviewer.get(); }

	void Update() override;
	void Render() override;

	void ChangeVSync(bool newIsEnabled);

	bool SupportsEditor() override { return false; }
};

template<typename GlRenderer>
[[nodiscard]] GLAssetManager* GetGLAssetManager(RendererObject<GlRenderer>* glRendererObjectContext)
{
	static_assert(std::is_base_of_v<GLRendererBase, GlRenderer>, "This call expects a Gl Renderer Object.");
	return Engine::GetRenderer(glRendererObjectContext)->GetGLAssetManager();
}

template<typename GlRenderer>
[[nodiscard]] GLPreviewer* GetGLPreviewer(RendererObject<GlRenderer>* glRendererObjectContext)
{
	static_assert(std::is_base_of_v<GLRendererBase, GlRenderer>, "This call expects a Gl Renderer Object.");
	return Engine::GetRenderer(glRendererObjectContext)->GetGLPreviewer();
}

} // namespace ogl
