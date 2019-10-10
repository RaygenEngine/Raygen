#pragma once

#include "renderer/Renderer.h"
#include "system/Engine.h"
#include "renderer/GenericGpuAssetManager.h"

namespace OpenGL {
struct GLAssetBase;

using GLAssetManager = GenericGpuAssetManager<OpenGL::GLAssetBase>;
} // namespace OpenGL


namespace OpenGL {
class GLRendererBase : public ObserverRenderer {
	HWND m_assochWnd{};
	HDC m_hdc{};
	HGLRC m_hglrc{};

	std::unique_ptr<GLAssetManager> m_glAssetManager;

	bool m_vsyncEnabled{ true };

public:
	GLRendererBase();

	virtual ~GLRendererBase();

	bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	void SwapBuffers() override;

	GLAssetManager* GetGLAssetManager() { return m_glAssetManager.get(); }

	virtual void Update() override;

	void ChangeVSync(bool newIsEnabled);
};

template<typename GlRenderer>
[[nodiscard]] GLAssetManager* GetGLAssetManager(RendererObject<GlRenderer>* glRendererObjectContext)
{
	static_assert(std::is_base_of_v<GLRendererBase, GlRenderer>, "This call expects a Gl Renderer Object.");
	return Engine::GetRenderer(glRendererObjectContext)->GetGLAssetManager();
}
} // namespace OpenGL
