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

	Engine::GetDrawReporter()->Reset();

	if (Engine::GetInput()->IsKeyPressed(Key::BACKSPACE)) {
		m_vsyncEnabled = !m_vsyncEnabled;
		ChangeVSync(m_vsyncEnabled);
	}

	if (Engine::GetInput()->IsKeyPressed(Key::P)) {
		m_previewerEnabled = !m_previewerEnabled;
	}

	if (m_previewerEnabled && Engine::GetInput()->IsKeyPressed(Key::K1)) {
		m_glPreviewer->PreviousPreview();
	}

	if (m_previewerEnabled && Engine::GetInput()->IsKeyPressed(Key::K2)) {
		m_glPreviewer->NextPreview();
	}
}

void GLRendererBase::Render()
{
	if (m_previewerEnabled) {
		m_glPreviewer->RenderPreview();
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
	CLOG_ERROR(!wglMakeCurrent(m_hdc, NULL), "Failed to make current");
	CLOG_ERROR(!wglDeleteContext(m_hglrc), "Failed to delete");
	CLOG_ERROR(ReleaseDC(m_assochWnd, m_hdc) != 1, "Failed to release");
	// gladUnloadGL();
	Engine::Get().m_remakeWindow = true;
}

void GLRendererBase::Init(HWND assochWnd, HINSTANCE instance)
{
	m_assochWnd = assochWnd;

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 2;
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
	auto dcc = wglGetCurrentContext();
	const int32 pixelFormat = ChoosePixelFormat(m_hdc, &pfd);

	CLOG_ABORT(!pixelFormat, "Can't find a suitable pixelFormat, error: {}", MB_OK | MB_ICONERROR);
	auto res = SetPixelFormat(m_hdc, pixelFormat, &pfd);
	if (!res) {
		// CLOG_ABORT(!res, "Can't set the pixelFormat, error: {}", GetLastError());
		LOG_ERROR("Using existing pixel format");
	}


	m_hglrc = wglCreateContext(m_hdc);

	CLOG_ABORT(!m_hglrc, "Can't create a GL rendering context, error: {}", MB_OK | MB_ICONERROR);
	res = wglMakeCurrent(m_hdc, m_hglrc);
	CLOG_ABORT(!res, "Can't activate GLRC, error: {}", MB_OK | MB_ICONERROR);
	res = gladLoadGL();
	CLOG_ABORT(!(res == 1), "Couldn't load ogl function pointers");
	m_glPreviewer->InitPreviewShaders(m_glAssetManager.get());

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	InitScene();
}

void GLRendererBase::SwapBuffers()
{
	wglMakeCurrent(m_hdc, NULL);
	::SwapBuffers(m_hdc);
	wglMakeCurrent(m_hdc, m_hglrc);
}


} // namespace ogl
