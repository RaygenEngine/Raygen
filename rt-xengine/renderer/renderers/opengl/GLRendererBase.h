#pragma once

#include "renderer/Renderer.h"
#include "renderer/renderers/opengl/GLAssetManager.h"


namespace OpenGL
{
	class GLRendererBase : public Renderer
	{
		HWND m_assochWnd;
		HDC m_hdc;
		HGLRC m_hglrc;

		GLAssetManager m_glAssetManager;
		
	public:
		GLRendererBase(Engine* engine);
		~GLRendererBase();

		bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
		void SwapBuffers() override;

		GLAssetManager* GetGLAssetManager() { return &m_glAssetManager; }
	};

}
