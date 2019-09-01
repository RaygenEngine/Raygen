#pragma once

#include "renderer/Renderer.h"
#include "renderer/renderers/opengl/GLAssetManager.h"


namespace Renderer::OpenGL
{
	class GLRendererBase : public Renderer
	{
		HWND m_assochWnd;
		HDC m_hdc;
		HGLRC m_hglrc;

		GLAssetManager m_glAssetManager;
		
	public:
		GLRendererBase(System::Engine* context);
		~GLRendererBase();

		bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
		void SwapBuffers() override;

		GLAssetManager* GetGLAssetManager() { return &m_glAssetManager; }

		void ToString(std::ostream& os) const override { os << "renderer-type: GLRendererBase, id: " << GetObjectId(); }
	};

}
