#pragma once

#include "renderer/Renderer.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "system/Engine.h"

namespace OpenGL
{
	class GLRendererBase : public Renderer
	{
		HWND m_assochWnd;
		HDC m_hdc;
		HGLRC m_hglrc;

		GLAssetManager m_glAssetManager;

	public:
		GLRendererBase();
		~GLRendererBase();

		bool InitRendering(HWND assochWnd, HINSTANCE instance) override;
		void SwapBuffers() override;

		GLAssetManager* GetGLAssetManager() 
		{
			return &m_glAssetManager;
		}
	};

	template<typename GlRenderer>
	GLAssetManager* GetGLAssetManager(RendererObject<GlRenderer>* context)
	{
		return Engine::GetRenderer<GLRendererBase>()->GetGLAssetManager();
		//return &Engine::GetRenderer<RendererObject<GLRendererBase>, GLRendererBase>(context)->m_glAssetManager;
	}
}
