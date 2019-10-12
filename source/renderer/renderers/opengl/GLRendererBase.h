#pragma once

#include "renderer/ObserverRenderer.h"
#include "renderer/GenericGpuAssetManager.h"
#include "system/Engine.h"

namespace ogl {
class GLAssetManager;
} // namespace ogl


namespace ogl {
class GLRendererBase : public ObserverRenderer {
	HWND m_assochWnd{ nullptr };
	HDC m_hdc{ nullptr };
	HGLRC m_hglrc{ nullptr };

	std::unique_ptr<GLAssetManager> m_glAssetManager;

	bool m_vsyncEnabled{ true };

public:
	GLRendererBase();

	virtual ~GLRendererBase();

	bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
	void SwapBuffers() override;

	GLAssetManager* GetGLAssetManager() { return m_glAssetManager.get(); }

	void Update() override;

	void ChangeVSync(bool newIsEnabled);

	bool SupportsEditor() override { return false; }
};

template<typename GlRenderer>
[[nodiscard]] GLAssetManager* GetGLAssetManager(RendererObject<GlRenderer>* glRendererObjectContext)
{
	static_assert(std::is_base_of_v<GLRendererBase, GlRenderer>, "This call expects a Gl Renderer Object.");
	return Engine::GetRenderer(glRendererObjectContext)->GetGLAssetManager();
}
} // namespace ogl
