#include "pch.h"

#include "renderer/renderers/opengl/GLRendererBase.h"

namespace Renderer::OpenGL
{

	GLRendererBase::GLRendererBase(System::Engine* context)
		: Renderer(context), m_assochWnd(nullptr), m_hdc(nullptr), m_hglrc(nullptr)
	{
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
		pfd.cColorBits = 16;
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
		pfd.cDepthBits = 16;
		pfd.cStencilBits = 0;
		pfd.cAuxBuffers = 0;
		pfd.iLayerType = PFD_MAIN_PLANE;
		pfd.bReserved = 0;
		pfd.dwLayerMask = 0;
		pfd.dwVisibleMask = 0;
		pfd.dwDamageMask = 0;

		m_hdc = GetDC(m_assochWnd);

		int32 pixelFormat = ChoosePixelFormat(m_hdc, &pfd);

		RT_XENGINE_ASSERT_RETURN_FALSE(pixelFormat, "Can't find a suitable pixelFormat, error: {}", MB_OK | MB_ICONERROR);
		RT_XENGINE_ASSERT_RETURN_FALSE(SetPixelFormat(m_hdc, pixelFormat, &pfd), "Can't set the pixelFormat, error: {}", MB_OK | MB_ICONERROR);

		m_hglrc = wglCreateContext(m_hdc);

		RT_XENGINE_ASSERT_RETURN_FALSE(m_hglrc, "Can't create a GL rendering context, error: {}", MB_OK | MB_ICONERROR);
		RT_XENGINE_ASSERT_RETURN_FALSE(wglMakeCurrent(m_hdc, m_hglrc), "Can't activate GLRC, error: {}", MB_OK | MB_ICONERROR);
		RT_XENGINE_ASSERT_RETURN_FALSE(gladLoadGL() == 1, "Couldn't load OpenGL function pointers, GL windows won't be loaded");

		return true;
	}

	void GLRendererBase::SwapBuffers()
	{
		// TODO: do i need this?
		//glFinish();
		::SwapBuffers(m_hdc);
	}

	std::shared_ptr<GLCubeMap> GLRendererBase::RequestGLCubeMap(Assets::CubeMap* cubeMap, GLint wrapFlag,
		bool mipMapping)
	{
		return Assets::LoadAssetAtMultiKeyCache<GLCubeMap>(m_glCubeMaps, this, cubeMap->GetName(), cubeMap, wrapFlag, mipMapping);
	}

	std::shared_ptr<GLTexture> GLRendererBase::RequestGLTexture(Assets::Texture* texture, GLint minFilter,
		GLint magFilter, GLint wrapS, GLint wrapT, GLint wrapR)
	{
		return Assets::LoadAssetAtMultiKeyCache<GLTexture>(m_glTextures, this, texture->GetName(), texture, minFilter, magFilter, wrapS, wrapT, wrapR);
	}

	std::shared_ptr<GLShader> GLRendererBase::RequestGLShader(Assets::StringFile* vertexFile, Assets::StringFile* fragmentFile)
	{
		const auto name = "vert> " + vertexFile->GetFileName() + "frag> " + fragmentFile->GetFileName();
		
		return Assets::LoadAssetAtMultiKeyCache<GLShader>(m_glShaders, this, name, vertexFile, fragmentFile);
	}

	std::shared_ptr<GLModel> GLRendererBase::RequestGLModel(Assets::Model* model)
	{
		return Assets::LoadAssetAtMultiKeyCache<GLModel>(m_glModels, this, model->GetName(), model);
	}

	// TODO: caching may not work correctly with instancing, check this out
	//std::shared_ptr<GLInstancedModel> GLRendererBase::RequestGLInstancedModel(World::TriangleModelInstancedGeometryNode* nodeInstancer)
	//{
	//	const auto name = nodeInstancer->GetModel()->GetName();
		
	//	return Assets::LoadAssetAtMultiKeyCache<GLInstancedModel>(m_glInstancedModels, this, name, nodeInstancer);
	//}
}
