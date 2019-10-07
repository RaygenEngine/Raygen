#include "pch.h"

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

#include "glad/glad.h"
#include "system/Input.h"

namespace OpenGL
{

void GLRendererBase::Update()
{
	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::BACKSPACE))
	{
		m_vsyncEnabled = !m_vsyncEnabled;
		ChangeVSync(m_vsyncEnabled);
	}
}

void GLRendererBase::ChangeVSync(bool enabled)
	{
		auto wglSwapIntervalEXT = ((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"));
		wglSwapIntervalEXT(enabled);
	}

	GLRendererBase::GLRendererBase()
		: m_assochWnd(nullptr),
		  m_hdc(nullptr),
		  m_hglrc(nullptr)
	{
		m_glAssetManager = std::make_unique<GLAssetManager>();
	}

	GLRendererBase::~GLRendererBase()
	{
		ChangeDisplaySettings(NULL, 0);
		wglMakeCurrent(m_hdc, NULL);
		wglDeleteContext(m_hglrc);
		ReleaseDC(m_assochWnd, m_hdc);
	}

	bool GLRendererBase::InitRendering(HWND assochWnd, HINSTANCE instance)
	{
		m_assochWnd = assochWnd;

		PIXELFORMATDESCRIPTOR pfd;
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cRedBits = 0;
		pfd.cRedShift = 0;
		pfd.cGreenBits = 0;
		pfd.cGreenShift = 0;
		pfd.cBlueBits = 0;
		pfd.cBlueShift = 0;
		pfd.cAlphaBits = 0;
		pfd.cAlphaShift = 0;
		pfd.cAccumBits = 0;
		pfd.cAccumRedBits = 0;
		pfd.cAccumGreenBits = 0;
		pfd.cAccumBlueBits = 0;
		pfd.cAccumAlphaBits = 0;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.cAuxBuffers = 0;
		pfd.iLayerType = PFD_MAIN_PLANE;
		pfd.bReserved = 0;
		pfd.dwLayerMask = 0;
		pfd.dwVisibleMask = 0;
		pfd.dwDamageMask = 0;

		m_hdc = GetDC(m_assochWnd);
		
		const int32 pixelFormat = ChoosePixelFormat(m_hdc, &pfd);

		if(!pixelFormat)
		{
			LOG_FATAL("Can't find a suitable pixelFormat, error: {}", MB_OK | MB_ICONERROR);
			return false;
		}
		
		if(!SetPixelFormat(m_hdc, pixelFormat, &pfd))
		{
			LOG_FATAL("Can't set the pixelFormat, error: {}", MB_OK | MB_ICONERROR);
			return false;
		}

		m_hglrc = wglCreateContext(m_hdc);

		if(!m_hglrc)
		{
			LOG_FATAL("Can't create a GL rendering context, error: {}", MB_OK | MB_ICONERROR);
			return false;
		}
		
		if(!wglMakeCurrent(m_hdc, m_hglrc))
		{
			LOG_FATAL("Can't activate GLRC, error: {}", MB_OK | MB_ICONERROR);
			return false;
		}
		
		if(!(gladLoadGL() == 1))
		{
			LOG_FATAL("Couldn't load OpenGL function pointers, GL windows won't be loaded");
			return false;
		};

		return true;
	}

	void GLRendererBase::SwapBuffers()
	{
		::SwapBuffers(m_hdc);
	}
	

}
