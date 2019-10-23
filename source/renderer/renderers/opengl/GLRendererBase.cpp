#include "pch/pch.h"

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/GLPreviewer.h"
#include "system/Input.h"

#include <glad/glad.h>


namespace ogl {

void GLRendererBase::Update()
{
	ObserverRenderer::Update();
	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::BACKSPACE)) {
		m_vsyncEnabled = !m_vsyncEnabled;
		ChangeVSync(m_vsyncEnabled);
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::P)) {
		m_previewerEnabled = !m_previewerEnabled;
	}

	if (m_previewerEnabled && Engine::GetInput()->IsKeyPressed(XVirtualKey::K1)) {
		m_glPreviewer->PreviousPreview();
	}

	if (m_previewerEnabled && Engine::GetInput()->IsKeyPressed(XVirtualKey::K2)) {
		m_glPreviewer->NextPreview();
	}
}

void GLRendererBase::ChangeVSync(bool enabled)
{
	auto wglSwapIntervalEXT = ((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"));
	wglSwapIntervalEXT(enabled);
}

GLRendererBase::GLRendererBase()
{
	m_glAssetManager = std::make_unique<GLAssetManager>();
	m_glPreviewer = std::make_unique<GLPreviewer>();
}

GLRendererBase::~GLRendererBase()
{
	ChangeDisplaySettings(NULL, 0);
	wglMakeCurrent(m_hdc, NULL);
	wglDeleteContext(m_hglrc);
	ReleaseDC(m_assochWnd, m_hdc);
}

void GLRendererBase::InitRendering(HWND assochWnd, HINSTANCE instance)
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

	CLOG_ABORT(!pixelFormat, "Can't find a suitable pixelFormat, error: {}", MB_OK | MB_ICONERROR);
	CLOG_ABORT(!SetPixelFormat(m_hdc, pixelFormat, &pfd), "Can't set the pixelFormat, error: {}", MB_OK | MB_ICONERROR);

	m_hglrc = wglCreateContext(m_hdc);

	CLOG_ABORT(!m_hglrc, "Can't create a GL rendering context, error: {}", MB_OK | MB_ICONERROR);
	CLOG_ABORT(!wglMakeCurrent(m_hdc, m_hglrc), "Can't activate GLRC, error: {}", MB_OK | MB_ICONERROR);
	CLOG_ABORT(!(gladLoadGL() == 1), "Couldn't load ogl function pointers");

	m_glPreviewer->InitPreviewShaders(m_glAssetManager.get());
}

void GLRendererBase::SwapBuffers()
{
	::SwapBuffers(m_hdc);
}


} // namespace ogl
